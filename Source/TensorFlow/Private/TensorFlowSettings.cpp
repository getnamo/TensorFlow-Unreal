#include "TensorFlowSettings.h"
#include "TensorFlow.h"
#include "PythonCLISettings.h"
#include "PythonEnvironment.h"
#include "PythonSubsystem.h"

UTensorFlowSettings::UTensorFlowSettings()
{
	SectionName = TEXT("TensorFlow");
}

FString UTensorFlowSettings::RequirementsFile(ETensorFlowProfile InProfile)
{
	const FString Dir = FTensorFlowModule::ScriptsDir() / TEXT("requirements");
	switch (InProfile)
	{
	case ETensorFlowProfile::TensorFlowCPU: return Dir / TEXT("tensorflow-cpu.txt");
	case ETensorFlowProfile::TensorFlowCUDA: return Dir / TEXT("tensorflow-cuda.txt");
	case ETensorFlowProfile::KerasTorch: return Dir / TEXT("keras-torch.txt");
	default: return FString();
	}
}

void UTensorFlowSettings::ApplyProfile() const
{
	for (uint8 i = 0; i <= (uint8)ETensorFlowProfile::None; ++i)
	{
		const FString File = RequirementsFile((ETensorFlowProfile)i);
		if (!File.IsEmpty())
		{
			FPythonEnvironment::UnregisterRequirementsFile(File);
		}
	}
	const FString File = RequirementsFile(Profile);
	if (!File.IsEmpty())
	{
		FPythonEnvironment::RegisterRequirementsFile(File);
	}
	FPythonEnvironment::RegisterEnvironmentVariable(TEXT("TF_CPP_MIN_LOG_LEVEL"), FString::FromInt(TensorFlowLogLevel));
	FPythonEnvironment::RegisterEnvironmentVariable(TEXT("KERAS_BACKEND"), Profile == ETensorFlowProfile::KerasTorch ? TEXT("torch") : FString());
	if (Profile == ETensorFlowProfile::KerasTorch)
	{
		UPythonCLISettings* Python = GetMutableDefault<UPythonCLISettings>();
		if (Python->TorchBackend.IsEmpty())
		{
			Python->TorchBackend = TEXT("auto"); //In memory only: uv picks torch's CUDA build for this GPU
		}
	}
}

#if WITH_EDITOR
void UTensorFlowSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplyProfile();
	if (UPythonSubsystem* Subsystem = UPythonSubsystem::Get())
	{
		Subsystem->SetupEnvironment(); //Installs the new profile in the background; running processes keep theirs
	}
}
#endif
