using UnrealBuildTool;

public class TFAudioCapture : ModuleRules
{
    public TFAudioCapture(ReadOnlyTargetRules Target) : base(Target)
    {
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
            new string[] {
                "TFAudioCapture/Public"
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                "TFAudioCapture/Private",
            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "CoreUtility"
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
            );
    }
}