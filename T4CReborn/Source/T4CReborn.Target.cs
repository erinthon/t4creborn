using UnrealBuildTool;
using System.Collections.Generic;

public class T4CRebornTarget : TargetRules
{
	public T4CRebornTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("T4CReborn");
	}
}
