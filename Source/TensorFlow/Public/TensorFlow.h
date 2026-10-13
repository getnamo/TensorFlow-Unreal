#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

TENSORFLOW_API DECLARE_LOG_CATEGORY_EXTERN(LogTensorFlow, Log, All);

class FTensorFlowModule : public IModuleInterface
{
public:
	/** Registers the plugin's Content/Scripts with PythonCLI (examples, TFPluginAPI) and the environment
	 profile from Project Settings > Plugins > TensorFlow. */
	virtual void StartupModule() override;

	/** The plugin's Content/Scripts folder. */
	static FString ScriptsDir();
};
