#include "TensorFlow.h"
#include "PythonEnvironment.h"
#include "TensorFlowSettings.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogTensorFlow);

FString FTensorFlowModule::ScriptsDir()
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("TensorFlow"));
	return Plugin.IsValid() ? FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir() / TEXT("Content/Scripts")) : FString();
}

void FTensorFlowModule::StartupModule()
{
	//Before the Python environment is set up (the engine subsystem initializes after Default-phase modules)
	FPythonEnvironment::RegisterScriptDirectory(ScriptsDir());
	GetDefault<UTensorFlowSettings>()->ApplyProfile();
}

IMPLEMENT_MODULE(FTensorFlowModule, TensorFlow)
