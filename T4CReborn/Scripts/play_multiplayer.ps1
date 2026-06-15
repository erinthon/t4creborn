# Lanca uma sessao multiplayer local de teste: 1 host (servidor-ouvinte) + 1 cliente.
# Uso: powershell -ExecutionPolicy Bypass -File Scripts\play_multiplayer.ps1
$ue   = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
$proj = Join-Path (Split-Path $PSScriptRoot -Parent) "T4CReborn.uproject"
$map  = "/Game/Maps/L_Arakas_Test"

# Host = jogador 1 (servidor-ouvinte), janela a esquerda
Start-Process -FilePath $ue -ArgumentList @("`"$proj`"", "$map`?listen", "-game", "-windowed", "-ResX=1100", "-ResY=720", "-WinX=10", "-WinY=40")
Write-Host "Host (servidor-ouvinte) lancado. Aguardando 12s para subir..."
Start-Sleep -Seconds 12

# Cliente = jogador 2, conecta no host local, janela a direita
Start-Process -FilePath $ue -ArgumentList @("`"$proj`"", "127.0.0.1", "-game", "-windowed", "-ResX=1100", "-ResY=720", "-WinX=760", "-WinY=120")
Write-Host "Cliente lancado. Controles: WASD mover | Mouse olhar | Botao esq./Espaco atacar."
