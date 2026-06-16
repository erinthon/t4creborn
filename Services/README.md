# Serviço de persistência — T4C: Reborn

Serviço HTTP que guarda os personagens num banco (SQLite), separado do servidor de jogo.
Mirror da arquitetura de produção: o servidor UE chama este serviço via HTTP; trocar o
armazenamento (Postgres, etc.) é mudar só este serviço — o jogo só enxerga a API HTTP.

Sem dependências externas: só a stdlib do Python 3 (`http.server`, `sqlite3`, `json`).

## Rodar

```powershell
python Services/persistence_service.py            # 127.0.0.1:8080, db em Services/characters.db
# ou
powershell -ExecutionPolicy Bypass -File T4CReborn\Scripts\run_persistence_service.ps1
```

## API

| Método | Rota | Corpo | Resposta |
|--------|------|-------|----------|
| GET | `/health` | — | `200 ok` |
| GET | `/character?id=<id>` | — | `200 {json}` ou `404` |
| PUT | `/character?id=<id>` | `{json}` | `204` ou `400` |

Exemplo:

```bash
curl -X PUT "http://127.0.0.1:8080/character?id=Hero" -H "Content-Type: application/json" -d '{"version":1,"hasChosenClass":true,"class":0,"level":2}'
curl "http://127.0.0.1:8080/character?id=Hero"
```

## Integração no jogo

- `UT4CPersistenceSubsystem` (UE) faz GET/PUT assíncrono; URL em `Config/DefaultGame.ini`
  (`[T4C.Persistence] BaseUrl`). Vazio desliga a persistência.
- `AT4CPlayerState::SerializeToJson`/`ApplyFromJson` montam/aplicam o JSON do personagem.
- Salva no `Logout` e por autosave (`AT4CGameMode`); carrega no 1º spawn do jogador.

`characters.db` e quaisquer artefatos do serviço NÃO são versionados.
