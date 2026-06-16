# Sobe o servico de persistencia (Python stdlib + SQLite) em 127.0.0.1:8080.
# Uso: powershell -ExecutionPolicy Bypass -File Scripts\run_persistence_service.ps1
$svc = Join-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) "Services\persistence_service.py"
Write-Host "Iniciando servico de persistencia: $svc"
python $svc --host 127.0.0.1 --port 8080
