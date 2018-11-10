// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "TensorFlow.h"
#include "TensorFlowPrivatePCH.h"
#include "UnrealEnginePython.h"
#include "IPluginManager.h"
#include "c_api.h"	//not currently used, but can be used

#define LOCTEXT_NAMESPACE "FTensorFlowModule"
DEFINE_LOG_CATEGORY(TensorFlowLog);

void FTensorFlowModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	//Get the python module and add self as a dependent
	if (FUnrealEnginePythonModule::IsAvailable())
	{
		FUnrealEnginePythonModule::Get().AddPythonDependentPlugin("TensorFlow");
	}
	else
	{
		UE_LOG(TensorFlowLog, Warning, TEXT("UnrealEnginePython Plugin not found! Make sure you've enabled it in Edit->Plugins."));
	}
}

void FTensorFlowModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTensorFlowModule, TensorFlow)