# T4C: Reborn

Um reimaginar **3D multiplayer** do clássico MMORPG **The 4th Coming (T4C / La Quatrième Prophétie, 1999)**, fiel à lore, ao mundo de **Althea** e às mecânicas originais — construído na **Unreal Engine 5**.

> Status: **Fase 0 — Pré-produção** (design + scaffolding). Engine ainda sendo instalada.

## O que é "fiel" aqui
Mantemos fielmente: o mundo de Althea e suas 3 ilhas (Arakas, Raven's Dust, Stoneheim), o sistema de 5 atributos (Força, Vigor, Agilidade, Inteligência, Sabedoria), as classes (Guerreiro, Magos elementais, Paladino, Battle Mage, Arqueiro, Clérigo, Ladino, Curandeiro), a progressão por pontos de atributo/perícia, e a profecia da "4ª vinda".
Reimaginamos: o visual 2D isométrico → **3D moderno** com câmera estilo action-RPG.

## Estrutura do repositório
```
t4c/
├─ README.md            ← este arquivo
├─ docs/
│  ├─ GDD.md            ← Game Design Document (mecânicas fiéis ao T4C)
│  ├─ ARCHITECTURE.md   ← Arquitetura técnica cliente-servidor UE5
│  └─ ROADMAP.md        ← Fases do desenvolvimento
└─ T4CReborn/           ← Projeto Unreal Engine 5 (C++)
   ├─ T4CReborn.uproject
   ├─ Config/
   ├─ Source/
   └─ Content/
```

## Pré-requisitos
- **Unreal Engine 5.6+** (via Epic Games Launcher)
- **Visual Studio 2022** com workload *Game development with C++* + *Desktop development with C++*
- Windows 10/11 64-bit

## Como abrir (depois da UE5 instalada)
1. Clique direito em `T4CReborn/T4CReborn.uproject` → **Generate Visual Studio project files**
2. Abra `T4CReborn.sln` no Visual Studio e compile (Development Editor | Win64)
3. Ou abra o `.uproject` direto — a engine pedirá para recompilar os módulos

## Testar multiplayer no editor
No editor: **Play → Net Mode → Play As Listen Server**, **Number of Players: 2**.
