// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;


public class TensorFlow : ModuleRules
{
    private string BinariesPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Binaries/")); }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
    }

    private string TensorflowThirdParty
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "Tensorflow")); }
    }
	private string ScriptsPath
	{
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Content/Scripts/")); }
	}

    public bool LoadLib(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
            string TensorLibPath = Path.Combine(TensorflowThirdParty, "Lib");
			string TensorflowLibDLL = Path.GetFullPath(Path.Combine(TensorLibPath, PlatformString, "tensorflow.dll"));

			RuntimeDependencies.Add(TensorflowLibDLL);
        }

        return isLibrarySupported;
    }


	public void AddScriptsAsDependencies(ReadOnlyTargetRules Target)
	{
		if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
		{
			RuntimeDependencies.Add(Path.Combine(ScriptsPath, "..."));
		}
	}

    public TensorFlow(ReadOnlyTargetRules Target) : base(Target)
    {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Private/TensorFlowPrivatePCH.h";

		PublicIncludePaths.AddRange(
			new string[] {
				"TensorFlow/Public"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"TensorFlow/Private",
                Path.Combine(TensorflowThirdParty, "Include"),
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "RHI",
                "RenderCore",
                "Engine",
                "UnrealEnginePython"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Projects"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        LoadLib(Target);
		AddScriptsAsDependencies(Target);

	}
}
