using UnrealBuildTool;

public class LambdaRunnable : ModuleRules
{
    public LambdaRunnable(TargetInfo Target)
    {

        PublicIncludePaths.AddRange(
            new string[] {
                "LambdaRunnable/Public"
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                "LambdaRunnable/Private",
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