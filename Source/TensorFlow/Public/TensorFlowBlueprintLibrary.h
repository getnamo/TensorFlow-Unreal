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
	
	/** Convert a UTexture2D to float array given the size of the texture*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToGrayScaleFloatArray (Texture2D)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static TArray<float> Conv_GreyScaleTexture2DToFloatArray(UTexture2D* InTexture);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToFloatArray (Texture2D)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static TArray<float> Conv_Texture2DToFloatArray(UTexture2D* InTexture);

	/** Invert values in a given float array (1->0, 0->1) on a 0-1 scale. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "InvertFloatArray"), Category = "Utilities|TensorFlow")
	static TArray<float> InvertFloatArray(const TArray<float>& InFloatArray);

	/** Convert a grayscale float array to a UTexture2D, defaults to square unless size param is specified*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTexture2D (Grayscale Array)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static UTexture2D* Conv_GrayScaleFloatArrayToTexture2D(const TArray<float>& InFloatArray, const FVector2D Size = FVector2D(0,0));

	/** Convert a float array to a UTexture2D, defaults to square unless size param is specified*/
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTexture2D (Float Array)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static UTexture2D* Conv_FloatArrayToTexture2D(const TArray<float>& InFloatArray, const FVector2D Size = FVector2D(0, 0));


	/** Convert render to target texture2d to a UTexture2D */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTexture2D (Render Target 2D)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static UTexture2D* Conv_RenderTargetTextureToTexture2D(UTextureRenderTarget2D* InTexture);

	/** Convert a byte array into a float array, normalized by the passed in scale */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToFloatArray (bytes)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static TArray<float> Conv_ByteToFloatArray(const TArray<uint8>& InByteArray, float Scale = 1.f);
};
