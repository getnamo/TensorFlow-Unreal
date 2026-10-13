#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TensorFlowSettings.generated.h"

/** Which packages the plugin adds to the project's Python environment. */
UENUM(BlueprintType)
enum class ETensorFlowProfile : uint8
{
	/** tensorflow-cpu 2.21. Windows has had no native TensorFlow GPU build since 2.10. */
	TensorFlowCPU UMETA(DisplayName = "TensorFlow (CPU)"),
	/** tensorflow[and-cuda] 2.21: Linux with an NVIDIA GPU (a few GB of CUDA wheels). */
	TensorFlowCUDA UMETA(DisplayName = "TensorFlow (CUDA, Linux)"),
	/** Keras 3 on the PyTorch backend, torch from the CUDA index uv detects: the way to a GPU on Windows.
	 Sets KERAS_BACKEND=torch, and Torch Backend "auto" unless the Python CLI settings name one. */
	KerasTorch UMETA(DisplayName = "Keras on PyTorch (GPU on Windows)"),
	/** Nothing: list your own packages in Project Settings > Plugins > Python CLI > Requirements. */
	None,
};

/** Project Settings > Plugins > TensorFlow. */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "TensorFlow"))
class TENSORFLOW_API UTensorFlowSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UTensorFlowSettings();

	/** Packages installed into the project's Python environment (installs are additive: switching profiles
	 keeps what's installed until the venv is deleted). */
	UPROPERTY(config, EditAnywhere, Category = "Environment")
	ETensorFlowProfile Profile = ETensorFlowProfile::TensorFlowCPU;

	/** TF_CPP_MIN_LOG_LEVEL for Python processes: 0 all, 1 no INFO, 2 no WARNING, 3 no ERROR. */
	UPROPERTY(config, EditAnywhere, Category = "Environment", meta = (ClampMin = "0", ClampMax = "3"))
	int32 TensorFlowLogLevel = 1;

	/** Register the profile's requirements and environment variables with PythonCLI. */
	void ApplyProfile() const;

	/** The requirements file of a profile (empty for None). */
	static FString RequirementsFile(ETensorFlowProfile InProfile);

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
