# Primeiros passos — quando a UE5 abrir

Sequência para sair do scaffold (C++) para uma cena multiplayer jogável.

## 0. Confirme a versão da engine
O `T4CReborn.uproject` está com `"EngineAssociation": "5.7"` (corresponde à 5.7.4 instalada).
Se trocar de versão depois, edite esse campo ou clique direito no `.uproject` → *Switch Unreal Engine version*.

## 1. Gerar e compilar
1. Clique direito em `T4CReborn/T4CReborn.uproject` → **Generate Visual Studio project files**.
2. Abra `T4CReborn.sln` → configuração **Development Editor / Win64** → **Build**.
3. A primeira compilação cria os módulos. Abra o `.uproject`.

> Se pedir para recompilar módulos ausentes ao abrir, aceite.

## 2. Criar o mapa de teste
1. **File → New Level → Basic** (tem chão + luz). Salve como `Content/Maps/L_Arakas_Test`.
2. Coloque alguns **Player Start** (2+) espalhados.
3. **Project Settings → Maps & Modes**: Editor Startup Map e Game Default Map = `L_Arakas_Test`.

## 3. Blueprint do personagem (para input)
O C++ deixa os campos de Enhanced Input expostos; ligamos no Blueprint:
1. Crie `IA_Move`, `IA_Look`, `IA_Attack` (Input Actions) e `IMC_Default` (Mapping Context)
   em `Content/Input/`. Em `IMC_Default`, mapeie WASD→`IA_Move`, Mouse XY→`IA_Look`, Botão esq.→`IA_Attack`.
2. Crie um Blueprint derivado de **AT4CCharacter** → `BP_T4CCharacter` (`Content/Blueprints/`).
3. No BP, em **Defaults**, atribua: Default Mapping Context = `IMC_Default`,
   Move/Look/Attack Action = os IA correspondentes. Dê um **Skeletal Mesh** + AnimBP
   (use o Manny/Quinn do template Third Person, ou cápsula placeholder).
4. **Project Settings → Maps & Modes → Default Pawn Class = `BP_T4CCharacter`**
   (ou aponte o GameMode para ele).

## 4. Testar multiplayer
Barra do **Play**:
- **Net Mode = Play As Listen Server**
- **Number of Players = 2**
- Play. Duas janelas; cada uma controla um personagem; ambos se veem mover em tempo real.

## 5. Verificar a espinha dorsal (já no C++)
- Movimento replicado ✔ (CharacterMovementComponent)
- HP derivado de Vigor no spawn ✔ (`PossessedBy` → `RecalculateDerivedStats`)
- Ataque autoritativo ✔ (`ServerAttack` → `ApplyDamage`)
- Morte concede XP ✔ (`HandleDeath` → `GrantExperience`) → level-up dá +5/+15 pontos
- HUD: ligar a delegate `OnHealthChanged` a um Widget UMG (próximo passo da Fase 1)

## Próximo (Fase 2)
DataTable `DT_GameBalance`, monstro IA, UI de distribuição de pontos chamando
`ServerAllocateStat`. Ver ROADMAP.md.
