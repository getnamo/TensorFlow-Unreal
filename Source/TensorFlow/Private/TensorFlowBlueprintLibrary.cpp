// Fill out your copyright notice in the Description page of Project Settings.

#include "TensorFlowPrivatePCH.h"
#include "RHI.h"
#include "TensorFlowBlueprintLibrary.h"


TArray<float> UTensorFlowBlueprintLibrary::Conv_Texture2DToFloatArray(UTexture2D* InTexture)
{
	TArray<float> FloatArray;
	FloatArray.SetNum(InTexture->GetSizeX()* InTexture->GetSizeY());

	// Lock the texture so it can be read
	uint8* MipData = static_cast<uint8*>(InTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));

	for (int i = 0; i < FloatArray.Num(); i++)
	{
		int MipPointer = i * 4;
		float GreyscaleValue = (MipData[MipPointer] + MipData[MipPointer + 1] + MipData[MipPointer + 2]) / 3.f;
		FloatArray[i] = 1 - (GreyscaleValue / 255.f);	//inverse and normalize it
	}

	// Unlock the texture
	InTexture->PlatformData->Mips[0].BulkData.Unlock();
	//InTexture->UpdateResource();

	return FloatArray;
}

UTexture2D* UTensorFlowBlueprintLibrary::Conv_FloatArrayToTexture2D(const TArray<float>& InFloatArray)
{
	//Create square image and lock for writing
	int32 Size = FMath::Pow(InFloatArray.Num(), 0.5);
	if (Size * Size != InFloatArray.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid float array, needs to be square."));
		return nullptr;
	}
	UTexture2D* Pointer = UTexture2D::CreateTransient(Size, Size, PF_R8G8B8A8);
	uint8* MipData = static_cast<uint8*>(Pointer->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	//Copy Data
	for (int i = 0; i < InFloatArray.Num(); i++)
	{
		int MipPointer = i * 4;
		int InverseValue = (1 - InFloatArray[i]) * 255.f;
		MipData[MipPointer] = InverseValue;
		MipData[MipPointer + 1] = InverseValue;
		MipData[MipPointer + 2] = InverseValue;
		MipData[MipPointer + 3] = 255;	//Alpha
	}

	//Unlock and Return data
	Pointer->PlatformData->Mips[0].BulkData.Unlock();
	Pointer->UpdateResource();
	return Pointer;
}

UTexture2D* UTensorFlowBlueprintLibrary::Conv_RenderTargetTextureToTexture2D(UTextureRenderTarget2D* PassedTexture)
{
	int TextureLength = PassedTexture->SizeX * PassedTexture->SizeY;
	UTexture2D* Pointer = UTexture2D::CreateTransient(PassedTexture->SizeX, PassedTexture->SizeY, PF_R8G8B8A8);
	
	TArray<FColor> SurfData;
	FRenderTarget *RenderTarget = PassedTexture->GameThread_GetRenderTargetResource();
	RenderTarget->ReadPixels(SurfData);

	uint8* MipData = static_cast<uint8*>(Pointer->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	//Copy Data
	for (int i = 0; i < TextureLength; i++)
	{
		int MipPointer = i * 4;
		const FColor& Color = SurfData[i];
		uint8 AdjustedColor = (Color.R + Color.G + Color.B) / 3;
		MipData[MipPointer] = AdjustedColor;
		MipData[MipPointer + 1] = AdjustedColor;
		MipData[MipPointer + 2] = AdjustedColor;
		MipData[MipPointer + 3] = 255;	//Alpha
	}

	//Unlock and Return data
	Pointer->PlatformData->Mips[0].BulkData.Unlock();
	Pointer->UpdateResource();
	PassedTexture->Source.UnlockMip(0);
	return Pointer;
}
