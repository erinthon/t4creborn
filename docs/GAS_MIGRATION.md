# RFC: Migração para o Gameplay Ability System (GAS)

> Fase 4 — Escala. Documento vivo; atualizado a cada estágio.

## Por quê

O combate caseiro (UT4CAttributeComponent à mão, habilidades por `switch`, cooldown/mana
manuais, predição de cliente improvisada) funciona para ~16 habilidades, mas não escala para
o volume de perícias/magias/itens de um MMO no espírito do T4C. O GAS é o padrão da indústria
para isso: replicação e predição prontas, GameplayEffects para buffs/DoTs/stacks, GameplayTags
para estados, e cada habilidade como uma classe isolada.

## Arquitetura (o "jeito real")

| Conceito | Decisão |
|---|---|
| ASC dos jogadores | No `AT4CPlayerState` (persiste entre respawns), `ReplicationMode = Mixed` |
| ASC dos monstros | No próprio pawn `AT4CMonster`, `ReplicationMode = Minimal` |
| Atributos | `UT4CAttributeSet`: primários (STR/END/AGI/INT/WIS), vitais (Health/MaxHealth/Mana/MaxMana), combate (Armor/WeaponDamageBonus), meta (IncomingDamage/IncomingHealing) |
| MaxHealth/MaxMana | Derivados via MMC (`UMMC_MaxHealth`/`UMMC_MaxMana`) + `GE_DerivedAttributes` infinito |
| Dano | `GE_Damage` (SetByCaller) + `UExec_Damage` (lê Armor do alvo, mín. 1) → meta IncomingDamage → Health em `PostGameplayEffectExecute` |
| Cura / Mana / Cooldown | GEs (`GE_Heal`, `GE_Cost`, `GE_Cooldown` com tag por slot) |
| Regen | `GE_Regen` infinito periódico (0.25s), aplicado no init |
| Habilidades | `UGameplayAbility`: `UGA_ProjectileAttack`/`UGA_Heal`/`UGA_Parry`; concedidas por classe |
| Equipamento | `GE_Equipment_Weapon`/`GE_Equipment_Armor` aplicados pelo inventário |
| Morte | delegate `OnT4CDeath` no AttributeSet → reaproveita XP/loot/respawn existentes |
| Tags | nativas em `GAS/T4CGameplayTags.{h,cpp}` |

Tudo em C++ puro (sem Blueprint), mantendo a convenção do projeto.

## Estágios

- **Estágio 0 — Fundação** ✅: plugin GameplayAbilities + módulos no Build.cs; tags nativas;
  pasta `GAS/`.
- **Estágio 1 — Atributos & vitais** ✅: ASC + AttributeSet; dano via `GE_Damage`+`UExec_Damage`,
  cura via `GE_Heal`, custo via `GE_Cost`, regen via `GE_Regen` periódico, derivados via
  `GE_DerivedAttributes`+MMCs. Morte via `IT4CCombatant`. `UT4CAttributeComponent` removido.
  Verificado ao vivo: paridade de combate (dano/morte/loot/level-up), sem erros.
  - *Simplificações temporárias (refinar nos próximos estágios):* cooldown ainda é manual
    (`LastAbilityTime`); equipamento aplica `SetNumericAttributeBase` direto (vira GE no Est. 3);
    Parry virou +Armadura plana temporária (era redução fracionária).
- **Estágio 2 — Habilidades** ✅: Q/E viram `UGameplayAbility` (`UGA_ProjectileAttack`/
  `UGA_Heal`/`UGA_Parry`, base `UT4CGameplayAbility` data-driven por `T4CAbilities::Get`).
  Custo via `GE_Cost`, cooldown via `GE_Cooldown` (SetByCaller duração + tag de slot
  dinâmica); `CheckCost/ApplyCost/CheckCooldown/ApplyCooldown` sobrescritos. Concedidas por
  classe no `ServerSelectClass` (InputID por slot); input ativa via
  `ASC->AbilityLocalInputPressed`. NetExecutionPolicy = ServerOnly (predição fica p/ depois).
  HUD de cooldown lê `GetActiveEffectsTimeRemaining` no ASC. Verificado por harness de
  auto-teste (`-T4CAutoTest` no GameMode): Fire Dart/FireStorm disparam com dano correto,
  cooldown bloqueia o 2º press, custo+regen de mana corretos, sem erros.
- **Estágio 3 — Equipamento & limpeza** ✅: equipamento via `GE_Equipment` infinito (Armadura
  e dano de arma por SetByCaller; aplicado/removido pelo `UT4CAbilitySystemComponent::ApplyEquipment`
  no equip/respawn). Parry agora é **redução fracionária** via o atributo `DamageReduction`
  (`GE_Parry` SetByCaller Data.Reduction), mitigada em `UExec_Damage` antes da armadura.
  Removido `SetEquipmentBonuses` (escrita direta de atributo). Verificado por auto-teste
  (Guerreiro equipa arma+10/armadura+7 → projétil dano 115; Parry alterna 50%↔0%), sem erros.

> **Migração GAS concluída.** Combate, atributos, habilidades, custo/cooldown, regen,
> equipamento e mitigação rodam inteiramente sobre o GAS. Refinamento futuro (não bloqueante):
> `NetExecutionPolicy = LocalPredicted` para predição de cliente; conceder a tag `State.Parrying`
> via `UTargetTagsGameplayEffectComponent` quando algum sistema for consumi-la; mover as tabelas
> de dados (classes/habilidades/itens/balance) para DataTables.

## Ferramentas

- **Auto-teste headless** (`-T4CAutoTest`): `AT4CGameMode` escolhe Mago para cada jogador e
  usa Q/E periodicamente — permite verificar o pipeline de combate sem input manual (o input
  por foco de janela é intermitente). Combine com `-LogCmds="LogTemp Verbose"`.

## Armadilhas conhecidas

- **Ordem de init**: `InitAbilityActorInfo` precisa rodar no servidor (`PossessedBy`) **e** no
  cliente (`OnRep_PlayerState`). Atributos inicializados antes de aplicar GEs.
- Mudança de módulo/plugin exige fechar o editor e regenerar project files.
- ASC no PlayerState: aumentar `NetUpdateFrequency` (já feito para progressão).
