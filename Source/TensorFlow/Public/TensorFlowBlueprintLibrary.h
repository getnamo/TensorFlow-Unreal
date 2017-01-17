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
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToFloatArray", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static TArray<float> Conv_Texture2DToFloatArray(UTexture2D* InTexture);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTexture2D", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static UTexture2D* Conv_FloatArrayToTexture2D(const TArray<float>& InFloatArray);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTexture2D (Render Target 2D)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static UTexture2D* Conv_RenderTargetTextureToTexture2D(UTextureRenderTarget2D* InTexture);
};
