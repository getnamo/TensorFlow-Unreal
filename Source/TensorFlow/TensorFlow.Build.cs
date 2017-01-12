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

    public bool LoadLib(TargetInfo Target)
    {
        bool isLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibrarySupported = true;

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
            string TensorLibPath = Path.Combine(TensorflowThirdParty, "Lib");

            //disable all this for now
            //PublicDelayLoadDLLs.Add("pywrap_tensorflow.dll");
            //RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(BinariesPath, "Win64", "pywrap_tensorflow.dll")));

            //doesn't load right for some reason =/
            //PublicAdditionalLibraries.Add(Path.Combine(TensorLibPath, PlatformString, "pywrap_tensorflow.lib"));
        }

        return isLibrarySupported;
    }


    public TensorFlow(TargetInfo Target)
	{

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
                "Engine"
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
    }
}
