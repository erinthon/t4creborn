using UnrealBuildTool;
using System.Collections.Generic;

// Alvo de Servidor Dedicado: roda headless (sem render/áudio), autoritativo.
// NOTA: o engine instalado pelo launcher (binário) NÃO compila targets Server
// ("Server targets are not currently supported from this engine distribution").
// Requer um engine compilado da fonte. A persistência (serviço HTTP + DB) não
// depende disto e roda igual no listen server, que também é autoritativo.
public class T4CRebornServerTarget : TargetRules
{
	public T4CRebornServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("T4CReborn");
	}
}
