using UnrealBuildTool;

public class TFAudioCapture : ModuleRules
{
	public TFAudioCapture(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AudioCaptureCore", // the default capture device on every platform (the AudioCapture plugin's backends)
		});
	}
}
