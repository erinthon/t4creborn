# T4C: Reborn — Roadmap

Marcos incrementais. Cada fase entrega algo jogável/verificável.

---

## Fase 0 — Pré-produção ✅ (em andamento)
- [x] Pesquisa das mecânicas fiéis do T4C
- [x] GDD + Arquitetura + Roadmap
- [x] Estrutura de pastas e scaffold do projeto UE5 (uproject, módulo, config, build)
- [ ] **Você:** instalar UE5 5.6+ e Visual Studio 2022 (workload C++)
- [ ] Gerar project files + primeira compilação limpa

## Fase 1 — Esqueleto multiplayer jogável
- [ ] Personagem 3D controlável (movimento replicado, câmera action-RPG)
- [ ] 2+ jogadores no mesmo mundo (listen server no editor)
- [ ] `AttributeComponent` com os 5 atributos + HP/Mana replicados
- [ ] HUD básico (HP/Mana/nível)
- [ ] **Critério:** dois personagens se veem e se movem em tempo real numa zona de teste

## Fase 2 — Combate e progressão
- [ ] Combate melee autoritativo (ataque → dano → morte)
- [ ] Monstro IA simples (spawn, perseguir, atacar)
- [ ] XP + level-up com distribuição de +5 atributo / +15 perícia
- [ ] DataTables de balanceamento (`DT_GameBalance`, `DT_Classes`)
- [ ] **Critério:** matar monstro, ganhar XP, subir de nível, ficar mais forte — replicado

## Fase 3 — Identidade T4C
- [x] Criação de personagem com "roll" de atributos (8 classes do GDD)
- [x] 1ª fatia de Arakas (vila + arredores) com layout fiel — placeholders: poço, casas, muralha, torre
- [x] Primeiras perícias como abilities (Powerful Blow, Parry) + 1 linha de magia (Fogo: Fire Dart)
- [x] Inventário + equipamento básico, loot — drop por raridade, coleta (F), auto-equip, poções (G)
- [x] **Critério:** criar um Guerreiro ou Mago e jogar a fatia vertical de Arakas com outro jogador

## Fase 4 — Escala
- [x] Migrar para Gameplay Ability System (GAS) — atributos, habilidades, custo/cooldown,
      dano/cura/regen, equipamento e parry como ASC/AttributeSet/GameplayEffect/GameplayAbility
      (ver `docs/GAS_MIGRATION.md`)
- [ ] Mais zonas (Raven's Dust), dungeons, mini-bosses
- [ ] NPCs de treino de perícia, lojas
- [ ] Dedicated server + persistência (DB)
- [ ] Evento global da "4ª Vinda"

---

### Riscos / notas
- **2D→3D:** assets 3D são o maior custo de produção; protótipo usa placeholders (cápsulas/primitivas + assets gratuitos do Marketplace/Quixel).
- **Fidelidade vs. modernização:** decisões de "fiel ao espírito" registradas no GDD; quando original e bom design moderno conflitarem, anotar a escolha.
- **Versão da engine:** fixar a versão da UE5 cedo evita quebras de API entre 5.x.
