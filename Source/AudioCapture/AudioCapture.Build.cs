using UnrealBuildTool;

public class AudioCapture : ModuleRules
{
    public AudioCapture(ReadOnlyTargetRules Target) : base(Target)
    {

        PublicIncludePaths.AddRange(
            new string[] {
                "AudioCapture/Public"
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                "AudioCapture/Private",
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