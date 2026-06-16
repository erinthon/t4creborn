# T4C: Reborn — Arquitetura técnica

Versão 0.1. Stack: **Unreal Engine 5.6+ (C++)**, replicação nativa cliente-servidor.

---

## 1. Modelo de rede: servidor autoritativo
Usamos o **modelo de replicação nativo da UE5** (dedicated/listen server autoritativo).
Regra de ouro: **o servidor decide tudo** que afeta gameplay (HP, dano, XP, posição validada). O cliente só prevê movimento e envia *input* via RPC `Server`.

```
[Cliente A] ──input/RPC──▶ [SERVIDOR autoritativo] ◀──input/RPC── [Cliente B]
     ▲  estado replicado          │  simula mundo           estado replicado  ▲
     └──────────────◀─────────────┴──────────────▶──────────────────────────┘
```

- **Dedicated Server** para produção; **Listen Server** para testes no editor (Play As Listen Server, 2 players).
- Movimento: `CharacterMovementComponent` (já tem predição + reconciliação do servidor).
- Ações de gameplay (atacar, conjurar, level-up): **RPCs `Server` + `Multicast`** e propriedades `Replicated`/`RepNotify`.

---

## 2. Ownership de dados (onde cada coisa vive)
| Dado | Classe UE | Replicação |
|------|-----------|------------|
| Progressão (nível, XP, pontos não-gastos, classe) + inventário + **ASC/AttributeSet** | `AT4CPlayerState` | Replicado; persiste entre respawns do pawn |
| Atributos de combate (STR/END/AGI/INT/WIS, HP/Mana, Armadura, etc.) | `UT4CAttributeSet` no ASC (jogador: PlayerState; monstro: pawn) | Replicado; só servidor escreve |
| Pawn/movimento | `AT4CCharacter` | Movimento replicado nativo |
| Input/câmera | `AT4CPlayerController` | Local; envia RPCs ao servidor |
| Regras de partida/spawn | `AT4CGameMode` (**só servidor**) | Não replicado |
| Estado global do mundo (evento da 4ª Vinda) | `AT4CGameState` | Replicado p/ todos |

---

## 3. Sistema de atributos — GAS (implementado na Fase 4)
O protótipo começou com um `UT4CAttributeComponent` simples e migrou para o **Gameplay Ability System** na Fase 4 (ver `docs/GAS_MIGRATION.md`). Hoje:

- **`UT4CAbilitySystemComponent`** — ASC; vive no `AT4CPlayerState` (jogadores, persiste entre respawns) e no pawn `AT4CMonster` (monstros).
- **`UT4CAttributeSet`** — STR/END/AGI/INT/WIS, HP/MaxHP, Mana/MaxMana, Armadura, bônus de arma, redução de dano e meta-atributos (IncomingDamage/Healing). MaxHP/MaxMana derivam dos primários via MMC.
- **`GameplayEffect`** (em C++) — dano (com Execution de mitigação), cura, custo de mana, cooldown, regeneração periódica, equipamento e parry. Magnitudes dinâmicas via SetByCaller.
- **`GameplayAbility`** — perícias/magias Q/E concedidas por classe; custo/cooldown como GEs.

> Decisão: começar simples (componente) reduziu risco no protótipo; GAS entrou assim que o número de habilidades/itens cresceu, evitando reescrever um volume grande de conteúdo.

---

## 4. Classes C++ do scaffold inicial
```
Source/T4CReborn/
├─ T4CReborn.h / .cpp          ← módulo primário
├─ Core/
│  ├─ T4CGameMode.h/.cpp       ← regras, spawn (só servidor)
│  ├─ T4CGameState.h/.cpp      ← estado global replicado (evento 4ª Vinda)
│  └─ T4CPlayerState.h/.cpp    ← atributos persistentes + nível/XP replicados
├─ Player/
│  ├─ T4CCharacter.h/.cpp      ← pawn 3D, combate melee básico
│  └─ T4CPlayerController.h/.cpp
└─ Attributes/
   ├─ T4CAttributeComponent.h/.cpp  ← HP/Mana/stats, dano, level-up (autoritativo)
   └─ T4CAttributeData.h            ← enum/struct dos 5 atributos + tabela de balance
```

---

## 5. Fluxos-chave (sequência)

### Aplicar dano (autoritativo)
1. Cliente A pressiona atacar → `ServerAttack()` RPC.
2. Servidor valida alcance/cooldown → calcula dano (`DanoMelee` do GDD).
3. Servidor chama `AttributeComponent->ApplyDamage()` no alvo (só servidor escreve HP).
4. `HP` é `Replicated` com `OnRep_Health` → todos os clientes atualizam HUD/feedback.
5. Se HP ≤ 0 → servidor processa morte + concede XP ao atacante.

### Level-up
1. XP cruza limiar no servidor → `GainLevel()`.
2. `+5` pontos de atributo e `+15` de perícia ficam pendentes no `PlayerState` (replicado).
3. Cliente abre UI, distribui, envia `ServerAllocateStat(Attr, amount)`.
4. Servidor valida pontos disponíveis → aplica → recalcula `MaxHP/MaxMana`.

---

## 6. Balanceamento orientado a dados
Constantes (`k_*`) e curvas de XP em **DataTables** (`Content/Data/`) — `DT_GameBalance`, `DT_Classes`, `DT_Skills`. Permite tunar sem recompilar C++.

---

## 7. Persistência (fase futura)
Protótipo: estado em memória do servidor (sem save). Futuro: backend (dedicated server + DB — Postgres/Redis) com perfil de personagem por conta. Fora de escopo agora.

---

## 8. Decisões de engenharia registradas
- **Servidor autoritativo desde o dia 1** — retrabalho de segurança/anti-cheat depois é caro.
- **C++ para core, Blueprint para conteúdo/UI** — lógica crítica e replicação em C++; designers iteram em BP derivados.
- **AttributeComponent antes de GAS** — pragmatismo no protótipo; caminho de migração documentado.
