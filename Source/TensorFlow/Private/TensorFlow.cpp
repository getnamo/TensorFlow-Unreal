// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "TensorFlowPrivatePCH.h"
#include "UnrealEnginePython.h"
#include "IPluginManager.h"
#include "c_api.h"

#define LOCTEXT_NAMESPACE "FTensorFlowModule"

void FTensorFlowModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	//Get the python module and try to add a path
	FString PluginRoot = IPluginManager::Get().FindPlugin("TensorFlow")->GetBaseDir();
	FString ScriptsPath = FPaths::Combine(PluginRoot, "Content/Scripts");
	FUnrealEnginePythonModule::Get().AddPathToSysPath(ScriptsPath);
	UE_LOG(LogTemp, Log, TEXT("Added Tensorflow Plugin Content/Scripts (%s) to sys.path"), *ScriptsPath);

	//Todo: handle and resolve pip dependencies from our json file
}

void FTensorFlowModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTensorFlowModule, TensorFlow)