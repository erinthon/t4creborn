# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

**T4C: Reborn** — a 3D multiplayer reimagining of the 1999 MMORPG *The 4th Coming*, built in **Unreal Engine 5.7.4** (C++). Faithful to the world of Althea, the 5 attributes (Strength/Endurance/Agility/Intelligence/Wisdom), the 8 classes, and point-based progression; reimagined from 2D isometric to 3D action-RPG.

The UE project lives in `T4CReborn/`. Design docs are in `docs/` (`GDD.md`, `ARCHITECTURE.md`, `ROADMAP.md`, `GAS_MIGRATION.md`). Note: `README.md` and `docs/ARCHITECTURE.md` describe the original pre-GAS prototype and are partly **outdated** — trust the source and `docs/GAS_MIGRATION.md` over them.

## Engine & toolchain (this machine)

- UE 5.7.4 at `C:\Program Files\Epic Games\UE_5.7`
- Visual Studio 2022 + MSVC 14.44; `EngineAssociation` = `"5.7"`
- `Target.cs` requires `BuildSettingsVersion.V6` (installed-engine binaries).

## Build

Build via UnrealBuildTool (reads `.uproject`/`Build.cs` directly — no need to regenerate the `.sln`). **The editor must be closed**, or the DLL link fails:

```bash
"C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" \
  T4CRebornEditor Win64 Development -Project="C:/dev/t4c/T4CReborn/T4CReborn.uproject" -WaitMutex
```

`-WarningsAsErrors` is on for UHT, so deprecation warnings fail the build. When changing modules/plugins (`.uproject`, `Build.cs`), close the editor first.

The module exposes its own root via `PublicIncludePaths.Add(ModuleDirectory)` in `Build.cs`, so includes are module-relative (`"GAS/T4CAttributeSet.h"`, `"Core/T4CGameMode.h"`) even though there is no Public/Private split.

## Run & verify (headless, no GUI clicking)

There is **no test suite**. Verification is done by launching real multiplayer sessions from the command line and grepping logs — the GUI cannot be driven here.

**Regenerate the test level** (only when the map/`Scripts/gen_test_level.py` changes) — run with a real RHI, **never `-nullrhi`** (it crashes div-by-zero):

```bash
"C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" \
  "C:/dev/t4c/T4CReborn/T4CReborn.uproject" \
  -ExecutePythonScript="C:/dev/t4c/T4CReborn/Scripts/gen_test_level.py" -unattended -nopause
```

**Launch a multiplayer session**: host = listen server, client connects to `127.0.0.1`. `Scripts/play_multiplayer.ps1` does this; for verification launch each with `-abslog="...host.log"` and `-game -windowed`. Map is `/Game/Maps/L_Arakas_Test`; add `?listen` for the host.

**Driving gameplay deterministically**: window-focus input is unreliable headless, so use the `-T4CAutoTest` flag — `AT4CGameMode` then auto-picks a class for each player, gives gear, and fires Q/E on a timer (also grants XP + saves once, for persistence tests). Combine with `-LogCmds="LogTemp Verbose"`. This is the primary way to exercise combat/abilities without a human.

**Persistence**: characters are saved via an external HTTP service (`Services/persistence_service.py`, Python stdlib + SQLite) — start it with `Scripts/run_persistence_service.ps1` (or `python Services/persistence_service.py`) before testing save/load. The game server talks to it through `UT4CPersistenceSubsystem` (URL in `Config/DefaultGame.ini` `[T4C.Persistence] BaseUrl`, **quoted** — UE's INI parser truncates an unquoted `//`). A true **dedicated server** (`T4CRebornServer.Target.cs`) needs a source-built engine — the launcher binary refuses Server targets; the listen server is the authoritative stand-in.

**Checking results**: grep the `Saved/Logs/*.log` for `[T4C]` markers (spawns, deaths, loot, level-ups, `[T4C][AutoTest]`), `Welcomed by server`/`Join succeeded` (connection), and absence of `Critical error`/`LogAbilitySystem: Error`.

## Architecture

**Server-authoritative.** Only the server mutates gameplay state; clients send input via `Server` RPCs and receive replicated state. Multiplayer is launched from the command line (no PIE), and assets/maps are generated headless via Python — there is no reliance on Blueprints or editor GUI. All gameplay, UI, and input are **pure C++** (no Blueprint assets):
- HUD is drawn with Canvas in `UI/T4CHUD.cpp` (no UMG).
- Input is classic action/axis mappings in `Config/DefaultInput.ini` (not Enhanced Input).

**Data ownership / persistence across respawn** — the key design axis. The pawn (`AT4CCharacter`) is destroyed and recreated on death/respawn, so anything that must persist lives on **`AT4CPlayerState`**: the inventory (`UT4CInventoryComponent`), progression (level/XP/primary stats), and the **Ability System Component**. Monsters (`AT4CMonster`) own their ASC on the pawn itself (no PlayerState).

**Gameplay Ability System (GAS)** is the combat backbone (migration tracked in `docs/GAS_MIGRATION.md`):
- `GAS/T4CAttributeSet` — primary attributes, vitals (Health/MaxHealth/Mana/MaxMana), combat (Armor/WeaponDamageBonus), and meta attributes (IncomingDamage/IncomingHealing). Clamping in `PreAttributeChange`; damage/heal/death applied in `PostGameplayEffectExecute`.
- `GAS/MMC_*` — MaxHealth/MaxMana derived reactively from primaries (captured with `bSnapshot=false`).
- `GAS/Effects/` — GameplayEffects defined **in C++ constructors** (no assets): `GE_DerivedAttributes`, `GE_Regen` (periodic), `GE_Damage` (+ `UExec_Damage` mitigates target Armor), `GE_Heal`, `GE_Cost`, `GE_Parry`, `GE_Cooldown`. Dynamic magnitudes use **SetByCaller** (`Data.*` tags). Avoid the deprecated `InheritableOwnedTagsContainer` (UE5.3+ moved granted tags to `UGameplayEffectComponent`); add tags via `Spec.DynamicGrantedTags` instead.
- `GAS/Abilities/` — `UT4CGameplayAbility` base + `UGA_ProjectileAttack`/`UGA_Heal`/`UGA_Parry`. Data-driven: params come from `T4CAbilities::Get(class, slot)`; the slot is derived from the spec's `InputID`. Cost/cooldown via overridden `Check/ApplyCost`/`Check/ApplyCooldown`. `NetExecutionPolicy = ServerOnly`.
- ASC init order is the classic footgun: `InitAbilityActorInfo` must run on the **server (`PossessedBy`) and client (`OnRep_PlayerState`)**, and attributes must be set before GEs are applied (see `T4CAbilitySystemComponent::ApplyStartupEffects`/`RefillVitals`).

**Death** is dispatched via the `IT4CCombatant` interface: `UT4CAttributeSet` detects lethal damage and calls `HandleDeath(Killer)` on the avatar actor, decoupling the AttributeSet (which may live on the PlayerState) from per-actor death logic (XP, loot, respawn). The killer is read from the effect context instigator.

**Data tables in code** (not yet UE DataTables): `Attributes/T4CClassData.h` (8 classes + attribute rolls), `Attributes/T4CAbilityData.h` (per-class Q/E loadout), `Items/T4CItemData.h` (loot table + rarity roll). `Attributes/T4CAttributeData.h` holds `FT4CPrimaryStats` (the roll source) and `FT4CBalanceConstants`.

**Combat rule**: never filter combat targets by concrete class — apply effects through the ASC of any actor that has one (`UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent`). Friendly fire is avoided by an explicit `IsA(AT4CCharacter)` check in `T4CProjectile`.

## Conventions

- Comments and log/UI strings are in **Portuguese (pt-BR)**; match the surrounding style.
- Log gameplay events with the `[T4C]` prefix on `LogTemp` so the headless verification grep finds them.
- Work happens on feature branches (e.g. `feature/gas-migration`), committed in verifiable stages; each commit builds and is exercised via a live session. Commits/PRs are co-authored per the global convention.
