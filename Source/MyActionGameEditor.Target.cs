using UnrealBuildTool;

public class MyActionGameEditorTarget : TargetRules
{
	public MyActionGameEditorTarget(TargetInfo Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		Type = TargetType.Editor;
		ExtraModuleNames.Add("MyActionGame");
	}
}
