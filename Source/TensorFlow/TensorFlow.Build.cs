using System.IO;
using UnrealBuildTool;

public class TensorFlow : ModuleRules
{
	public TensorFlow(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"DeveloperSettings",
			"PythonCLI",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Projects",
			"Json",
			"ImageCore",
			"RenderCore",
			"RHI",
		});

		// The bundled scripts (examples, environment profiles, TFPluginAPI) go with packaged builds.
		RuntimeDependencies.Add(Path.Combine(PluginDirectory, "Content", "Scripts", "..."));
	}
}
