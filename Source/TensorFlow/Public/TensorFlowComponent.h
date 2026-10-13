#pragma once

#include "CoreMinimal.h"
#include "PythonMLComponent.h"
#include "TensorFlowBlueprintLibrary.h"
#include "TensorFlowComponent.generated.h"

class UTexture2D;
class UTextureRenderTarget2D;

/**
 * Runs a TensorFlow / Keras script: Script Params > Script names it ("my_model" or "my_model.py" under
 * Content/Scripts; bundled examples such as "examples/mnist_keras_cnn" also resolve). The script's get_api()
 * returns a TFPluginAPI (or unreal_bridge.ml.MLPluginAPI) subclass; see UPythonMLComponent for inputs,
 * training and events. Scripts run out of process in the shared Python process by default.
 */
UCLASS(ClassGroup = "Python", meta = (BlueprintSpawnableComponent, DisplayName = "TensorFlow"))
class TENSORFLOW_API UTensorFlowComponent : public UPythonMLComponent
{
	GENERATED_BODY()
public:
	/** A texture as a float32 (H, W, C) tensor to on_tensor_input (or FunctionName). Result: OnInputResults. */
	UFUNCTION(BlueprintCallable, Category = "TensorFlow", meta = (AutoCreateRefTerm = "Options,FunctionName"))
	void SendTextureInput(UTexture2D* Texture, const FTensorFlowImageOptions& Options, const FString& FunctionName);

	/** As SendTextureInput, reading the render target now (blocking). For every frame, use a Python Render
	 Target Stream instead. */
	UFUNCTION(BlueprintCallable, Category = "TensorFlow", meta = (AutoCreateRefTerm = "Options,FunctionName"))
	void SendRenderTargetInput(UTextureRenderTarget2D* RenderTarget, const FTensorFlowImageOptions& Options, const FString& FunctionName);

	/** 1.x: {"pixels": [grayscale 0..1 values], "size": {"x": W, "y": H}} to on_json_input (or FunctionName). */
	UFUNCTION(BlueprintCallable, Category = "TensorFlow", meta = (AutoCreateRefTerm = "FunctionName"))
	void SendJsonInputTexture(UTexture2D* Texture, const FString& FunctionName);
};
