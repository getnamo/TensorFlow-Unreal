using UnrealBuildTool;

public class CoreUtility : ModuleRules
{
    public CoreUtility(TargetInfo Target)
    {

        PublicIncludePaths.AddRange(
            new string[] {
                "CoreUtility/Public"
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                "CoreUtility/Private",
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
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
            );
    }
}