# T4C: Reborn — Game Design Document

Documento vivo. Versão 0.1 — Pré-produção.
Objetivo: reimaginar *The 4th Coming* (1999) em **3D multiplayer** na UE5, **fiel à lore e às mecânicas** do original.

---

## 1. Pilares de design (o que torna isto "T4C")
1. **Althea fiel** — geografia, cidades, dungeons e lore das 3 ilhas preservadas.
2. **Build por atributos** — os mesmos 5 atributos governam tudo; o jogador "rola" e distribui pontos, definindo a classe pela alocação (classe é emergente, não rígida — como no original).
3. **Progressão lenta e significativa** — XP por matar monstros; cada nível dá pontos de atributo + perícia para treinar com NPCs.
4. **Mundo compartilhado** — outros jogadores são reais; grupos, troca, e PvP em zonas designadas.
5. **A 4ª Vinda** — a profecia como espinha dorsal narrativa e gatilho de eventos de mundo.

---

## 2. Atributos (fiel ao original)
Cinco atributos centrais. Ao subir de nível: **+5 pontos de atributo** e **+15 pontos de perícia** (treináveis com NPCs).

| Atributo | Sigla | Governa (reimaginado, fiel ao papel original) |
|----------|-------|------------------------------------------------|
| **Força** (Strength) | STR | Dano corpo-a-corpo, capacidade de carga, requisito de armas/armaduras pesadas |
| **Vigor** (Endurance) | END | **HP ganho por nível**, regeneração, resistência física |
| **Agilidade** (Agility) | AGI | Velocidade de ataque, chance de acerto/esquiva, perícias de combate, velocidade de tiro do arqueiro |
| **Inteligência** (Intelligence) | INT | Poder de magia ofensiva, mana, eficácia de feitiços de dano |
| **Sabedoria** (Wisdom) | WIS | Mana, poder de cura/proteção, requisito de feitiços divinos |

**Fórmulas (primeira iteração — ajustável):**
- `MaxHP = HP_base + Σ(END_no_nível × k_hp)` — HP é concedido **no momento** do level-up conforme END atual (fiel ao original: END alto cedo = mais HP acumulado).
- `MaxMana = base + INT × k_int_mana + WIS × k_wis_mana`
- `DanoMelee = arma × (1 + STR × k_str)`
- `DanoMagia = base_feitiço × (1 + INT × k_int_dmg)`
- `Cura = base_feitiço × (1 + WIS × k_wis_heal)`
- `IntervaloDeAtaque = clamp(base_atk − AGI × k_agi_speed, min, base)`

> Constantes `k_*` ficam em uma *data table* (`DT_GameBalance`) para tuning sem recompilar.

---

## 3. Classes (alocação fiel ao T4C Bible)
Classe = perfil de atributos + perícias treinadas. Recomendações de "roll" inicial herdadas do original:

| Classe | Stats primários | Roll de referência | Estilo |
|--------|-----------------|--------------------|--------|
| **Guerreiro** | STR, END, AGI | 22 STR / 22 END / 15+ AGI | Melee bruto, armadura pesada |
| **Mago** (elemental) | INT, WIS | 20+ INT / 20+ WIS / 14+ END | Conjurador à distância (Fogo/Água/Terra/Ar/Trevas/Luz/Necromancia) |
| **Paladino** | STR, END, WIS | 20+ STR / 18+ END / 18+ WIS | Híbrido melee + cura/proteção |
| **Battle Mage** | INT, STR, AGI, END | 20+ INT / 14+ STR / 14+ END | Conjurador que luta com arma |
| **Arqueiro** | AGI, STR, END | 20+ AGI / 18+ STR / 20+ END | Dano à distância, tiros rápidos |
| **Clérigo** | WIS, END | 22 WIS / 16+ END | Suporte/tank de grupo |
| **Ladino** | AGI, INT, WIS | 20+ AGI / 14+ INT / 14+ WIS | Furtividade, roubo |
| **Curandeiro** | WIS, INT, END | 20+ WIS / 15+ END | Suporte puro de cura |

---

## 4. Perícias e Magias
**Perícias de combate (base AGI):** Stun Blow, Parry, Powerful Blow, Armor Penetration.
**Magias (exemplos fiéis):** Fire Dart (inicial), FireStorm (Mago de Fogo nível 27), linhas de Água/Terra/Ar/Trevas/Luz/Necromancia.

Modelo de skill: cada perícia/magia é um *Gameplay Ability* (ver ARCHITECTURE.md) com requisito mínimo de atributo e custo (mana/stamina). Treinada com NPC gastando pontos de perícia.

---

## 5. Mundo — Althea (3 ilhas)
| Ilha | Papel | Locais-chave |
|------|-------|--------------|
| **Arakas** | Zona inicial (níveis baixos) | Vila inicial, campos de treino |
| **Raven's Dust** | Mid-game | City of Silversky, Anrak's House, Bane's Island → Bane's Dungeon (Skullkeep) |
| **Stoneheim** | End-game | Centaur Village, Oracle Realm, Centaur Cave, The Colosseum, Thieves Hideout, Undead Cave, Wasp Cave |

40 mini-bosses espalhados pelas ilhas + Oracle Realm. A **profecia da 4ª Vinda** dispara eventos globais.

**Para o protótipo:** uma única zona jogável de Arakas (vila + arredores com monstros) — fatia vertical multiplayer.

---

## 6. Loop de gameplay (protótipo)
1. Criar personagem → distribuir atributos iniciais (roll).
2. Entrar em Arakas (servidor compartilhado).
3. Matar monstros → ganhar XP → subir de nível → alocar +5 atributo / +15 perícia.
4. Equipar loot, treinar 1–2 perícias com NPC.
5. Ver outros jogadores em tempo real; agrupar.

**Critério de sucesso do protótipo:** 2+ jogadores conectados no mesmo mundo, com movimento, combate, dano/morte, ganho de XP e level-up — tudo replicado e autoritativo no servidor.

---

## 7. Fora de escopo no protótipo (fases futuras)
Economia/lojas completas, crafting, todas as 8 classes balanceadas, PvP, dungeons instanciadas, persistência em banco de dados, todas as 3 ilhas. Ver ROADMAP.md.
