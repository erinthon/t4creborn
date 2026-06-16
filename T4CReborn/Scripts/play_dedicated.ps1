# Sobe servidor + cliente conectado, para testar a topologia dedicada.
#
# NOTA: o servidor dedicado de verdade (T4CRebornServer.exe) exige um engine
# compilado da FONTE — o engine instalado pelo launcher nao compila targets Server
# ("Server targets are not currently supported from this engine distribution").
# Enquanto isso, usamos um listen server headless (tambem autoritativo) como
# stand-in; o codigo de servidor (GameMode/PlayerState/persistencia) e identico.
#
# Suba o servico de persistencia antes (run_persistence_service.ps1).
# Uso: powershell -ExecutionPolicy Bypass -File Scripts\play_dedicated.ps1
$ue   = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
$proj = Join-Path (Split-Path $PSScriptRoot -Parent) "T4CReborn.uproject"
$map  = "/Game/Maps/L_Arakas_Test"

$server = Join-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) "T4CReborn\Binaries\Win64\T4CRebornServer.exe"
if (Test-Path $server) {
    # Servidor dedicado real (quando compilado num engine de fonte).
    Start-Process -FilePath $server -ArgumentList @("$map", "-log", "-port=7777")
} else {
    # Stand-in: listen server headless.
    Start-Process -FilePath $ue -ArgumentList @("`"$proj`"", "$map`?listen", "-server", "-log", "-nullrhi")
}
Write-Host "Servidor lancado. Aguardando 12s..."
Start-Sleep -Seconds 12

Start-Process -FilePath $ue -ArgumentList @("`"$proj`"", "127.0.0.1", "-game", "-windowed", "-ResX=1100", "-ResY=720")
Write-Host "Cliente lancado (conecta em 127.0.0.1)."
