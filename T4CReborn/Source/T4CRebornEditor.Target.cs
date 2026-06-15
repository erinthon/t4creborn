using UnrealBuildTool;
using System.Collections.Generic;

public class T4CRebornEditorTarget : TargetRules
{
	public T4CRebornEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("T4CReborn");
	}
}
