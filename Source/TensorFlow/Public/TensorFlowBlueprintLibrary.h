// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "TensorFlowBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TENSORFLOW_API UTensorFlowBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	/** Convert a UTexture2D to float array, assuming a square texture*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToFloatArray", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static TArray<float> Conv_Texture2DToFloatArray(UTexture2D* InTexture);

	/** Convert a float array to a UTexture2D, assuming square array */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTexture2D", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static UTexture2D* Conv_FloatArrayToTexture2D(const TArray<float>& InFloatArray);

	/** Convert render to target texture2d to a UTexture2D */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTexture2D (Render Target 2D)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static UTexture2D* Conv_RenderTargetTextureToTexture2D(UTextureRenderTarget2D* InTexture);

	/** Convert a byte array into a float array, normalized by the passed in scale */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToFloatArray (byte)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static TArray<float> Conv_ByteToFloatArray(const TArray<uint8>& InByteArray, float Scale = 1.f);
};
