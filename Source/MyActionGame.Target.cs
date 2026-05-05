using UnrealBuildTool;

public class MyActionGameTarget : TargetRules
{
	public MyActionGameTarget(TargetInfo Target) : base(Target)
	{
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		Type = TargetType.Game;
		ExtraModuleNames.Add("MyActionGame");
	}
}
