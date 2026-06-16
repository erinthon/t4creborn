"""
T4C: Reborn — Serviço de persistência de personagens.

Arquitetura de produção (prototipo): o servidor de jogo (UE) NAO fala com o banco
direto; ele chama este serviço HTTP, que guarda os personagens num DB (SQLite).
Trocar SQLite por Postgres/qualquer store é mudar só este arquivo — o jogo só vê HTTP.

Sem dependências externas: usa apenas a stdlib do Python 3 (http.server, sqlite3, json).

Rotas:
  GET  /health             -> 200 "ok"
  GET  /character?id=<id>  -> 200 {json do personagem} | 404
  PUT  /character?id=<id>  -> upsert (corpo = json) -> 204 | 400

Uso:
  python persistence_service.py [--host 127.0.0.1] [--port 8080] [--db characters.db]
"""
import argparse
import json
import os
import sqlite3
import threading
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlparse, parse_qs

DB_LOCK = threading.Lock()
DB_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "characters.db")


def now_iso():
    return datetime.now(timezone.utc).isoformat()


def init_db(path):
    conn = sqlite3.connect(path)
    conn.execute(
        """CREATE TABLE IF NOT EXISTS characters (
               id         TEXT PRIMARY KEY,
               version    INTEGER NOT NULL DEFAULT 1,
               json       TEXT    NOT NULL,
               updated_at TEXT    NOT NULL
           )"""
    )
    conn.commit()
    conn.close()


def db_get(path, char_id):
    with DB_LOCK:
        conn = sqlite3.connect(path)
        try:
            row = conn.execute("SELECT json FROM characters WHERE id = ?", (char_id,)).fetchone()
            return row[0] if row else None
        finally:
            conn.close()


def db_put(path, char_id, body_text):
    # Valida que o corpo é JSON antes de gravar (o store guarda JSON estruturado).
    parsed = json.loads(body_text)
    version = int(parsed.get("version", 1)) if isinstance(parsed, dict) else 1
    with DB_LOCK:
        conn = sqlite3.connect(path)
        try:
            conn.execute(
                """INSERT INTO characters (id, version, json, updated_at)
                   VALUES (?, ?, ?, ?)
                   ON CONFLICT(id) DO UPDATE SET
                       version=excluded.version, json=excluded.json, updated_at=excluded.updated_at""",
                (char_id, version, body_text, now_iso()),
            )
            conn.commit()
        finally:
            conn.close()


def log(msg):
    print(f"[{now_iso()}] {msg}", flush=True)


class Handler(BaseHTTPRequestHandler):
    db_path = DB_PATH

    def _send(self, code, body=b"", content_type="application/json"):
        self.send_response(code)
        if body:
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        if body:
            self.wfile.write(body)

    def _char_id(self):
        qs = parse_qs(urlparse(self.path).query)
        ids = qs.get("id", [])
        return ids[0] if ids else None

    def do_GET(self):
        route = urlparse(self.path).path
        if route in ("/", "/health"):
            self._send(200, b"ok", "text/plain")
            return
        if route == "/character":
            char_id = self._char_id()
            if not char_id:
                self._send(400, b'{"error":"missing id"}')
                return
            data = db_get(self.db_path, char_id)
            if data is None:
                log(f"GET  {char_id} -> 404")
                self._send(404, b'{"error":"not found"}')
            else:
                log(f"GET  {char_id} -> 200")
                self._send(200, data.encode("utf-8"))
            return
        self._send(404, b'{"error":"unknown route"}')

    def do_PUT(self):
        if urlparse(self.path).path != "/character":
            self._send(404, b'{"error":"unknown route"}')
            return
        char_id = self._char_id()
        if not char_id:
            self._send(400, b'{"error":"missing id"}')
            return
        length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(length).decode("utf-8") if length else ""
        try:
            db_put(self.db_path, char_id, body)
        except (json.JSONDecodeError, ValueError) as e:
            log(f"PUT  {char_id} -> 400 ({e})")
            self._send(400, b'{"error":"invalid json"}')
            return
        log(f"PUT  {char_id} -> 204 ({length} bytes)")
        self._send(204)

    # Silencia o log padrão (usamos o nosso).
    def log_message(self, *args):
        pass


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=8080)
    ap.add_argument("--db", default=DB_PATH)
    args = ap.parse_args()

    init_db(args.db)
    Handler.db_path = args.db

    server = ThreadingHTTPServer((args.host, args.port), Handler)
    log(f"T4C persistence service em http://{args.host}:{args.port}  (db={args.db})")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        log("encerrando")
        server.shutdown()


if __name__ == "__main__":
    main()
