#include "TensorFlowBlueprintLibrary.h"
#include "TensorFlow.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageCore.h"
#include "TextureResource.h"

namespace TensorFlowImagePrivate
{
	static FLinearColor FromBytes(const FColor& C)
	{
		return FLinearColor(C.R / 255.f, C.G / 255.f, C.B / 255.f, C.A / 255.f);
	}

	static UTexture2D* MakeTexture(int32 Width, int32 Height, TFunctionRef<FColor(int32 Index)> Pixel)
	{
		if (Width <= 0 || Height <= 0)
		{
			return nullptr;
		}
		UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
		if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
		{
			return nullptr;
		}
		FColor* Mip = static_cast<FColor*>(Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE)); //BGRA in memory
		for (int32 i = 0, N = Width * Height; i < N; ++i)
		{
			Mip[i] = Pixel(i);
		}
		Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
		Texture->UpdateResource();
		return Texture;
	}

	static uint8 ToByte(float Value)
	{
		return (uint8)FMath::Clamp(FMath::RoundToInt(Value * 255.f), 0, 255);
	}

	/** Square side for Count values, or 0. */
	static int32 SquareSide(int32 Count)
	{
		const int32 Side = FMath::RoundToInt(FMath::Sqrt((float)Count));
		return Side * Side == Count ? Side : 0;
	}

	static FLinearColor Sample(const TArray<FLinearColor>& Pixels, int32 Width, int32 Height, float X, float Y)
	{
		const float Fx = FMath::Clamp(X, 0.f, (float)(Width - 1));
		const float Fy = FMath::Clamp(Y, 0.f, (float)(Height - 1));
		const int32 X0 = FMath::FloorToInt(Fx), Y0 = FMath::FloorToInt(Fy);
		const int32 X1 = FMath::Min(X0 + 1, Width - 1), Y1 = FMath::Min(Y0 + 1, Height - 1);
		const float Tx = Fx - X0, Ty = Fy - Y0;
		const FLinearColor Top = FMath::Lerp(Pixels[Y0 * Width + X0], Pixels[Y0 * Width + X1], Tx);
		const FLinearColor Bottom = FMath::Lerp(Pixels[Y1 * Width + X0], Pixels[Y1 * Width + X1], Tx);
		return FMath::Lerp(Top, Bottom, Ty);
	}
}

bool UTensorFlowBlueprintLibrary::ReadTexture(UTexture2D* Texture, TArray<FLinearColor>& OutPixels, int32& OutWidth, int32& OutHeight, FString& OutError)
{
	using namespace TensorFlowImagePrivate;
	OutPixels.Reset();
	if (!Texture)
	{
		OutError = TEXT("no texture");
		return false;
	}

	//1. The editor's source image, 2. a CPU copy (Availability: CPU). Both as the stored 8-bit values.
	FImage Image;
	bool bImage = false;
#if WITH_EDITORONLY_DATA
	bImage = Texture->Source.IsValid() && Texture->Source.GetMipImage(Image, 0, 0, 0);
#endif
	if (!bImage)
	{
		const FSharedImageConstRef Copy = Texture->GetCPUCopy();
		if (Copy.IsValid())
		{
			Copy->CopyTo(Image);
			bImage = true;
		}
	}
	if (bImage)
	{
		FImage Bytes;
		Image.CopyTo(Bytes, ERawImageFormat::BGRA8, Image.GammaSpace == EGammaSpace::Linear ? EGammaSpace::Linear : EGammaSpace::sRGB);
		OutWidth = (int32)Bytes.SizeX;
		OutHeight = (int32)Bytes.SizeY;
		const TArrayView64<FColor> Colors = Bytes.AsBGRA8();
		OutPixels.Reserve(Colors.Num());
		for (const FColor& C : Colors)
		{
			OutPixels.Add(FromBytes(C));
		}
		return true;
	}

	//3. Uncompressed platform data still in memory (e.g. transient textures, UserInterface2D / Grayscale)
	FTexturePlatformData* Data = Texture->GetPlatformData();
	if (Data && Data->Mips.Num() > 0)
	{
		FTexture2DMipMap& Mip = Data->Mips[0];
		const EPixelFormat Format = Data->PixelFormat;
		const int32 Bpp = (Format == PF_B8G8R8A8 || Format == PF_R8G8B8A8) ? 4 : (Format == PF_G8 ? 1 : 0);
		if (Bpp > 0 && Mip.BulkData.GetBulkDataSize() >= (int64)Mip.SizeX * Mip.SizeY * Bpp)
		{
			const uint8* Src = static_cast<const uint8*>(Mip.BulkData.Lock(LOCK_READ_ONLY));
			if (Src)
			{
				OutWidth = Mip.SizeX;
				OutHeight = Mip.SizeY;
				OutPixels.SetNumUninitialized(OutWidth * OutHeight);
				for (int32 i = 0; i < OutPixels.Num(); ++i)
				{
					const uint8* P = Src + (int64)i * Bpp;
					OutPixels[i] = Bpp == 1 ? FromBytes(FColor(P[0], P[0], P[0], 255))
						: Format == PF_B8G8R8A8 ? FromBytes(FColor(P[2], P[1], P[0], P[3])) : FromBytes(FColor(P[0], P[1], P[2], P[3]));
				}
			}
			Mip.BulkData.Unlock();
			if (Src)
			{
				return true;
			}
		}
	}
	OutError = FString::Printf(TEXT("texture %s can't be read on the CPU: set its Availability to CPU, or its Compression to UserInterface2D or Grayscale (uncompressed), or render it to a render target"),
		*Texture->GetName());
	return false;
}

bool UTensorFlowBlueprintLibrary::ReadRenderTarget(UTextureRenderTarget2D* RenderTarget, TArray<FLinearColor>& OutPixels, int32& OutWidth, int32& OutHeight, FString& OutError)
{
	using namespace TensorFlowImagePrivate;
	OutPixels.Reset();
	FTextureRenderTargetResource* Resource = RenderTarget ? RenderTarget->GameThread_GetRenderTargetResource() : nullptr;
	if (!Resource)
	{
		OutError = TEXT("no render target (or it has no resource yet)");
		return false;
	}
	OutWidth = RenderTarget->SizeX;
	OutHeight = RenderTarget->SizeY;
	const EPixelFormat Format = RenderTarget->GetFormat();
	if (Format == PF_B8G8R8A8 || Format == PF_R8G8B8A8)
	{
		TArray<FColor> Colors;
		if (!Resource->ReadPixels(Colors))
		{
			OutError = TEXT("reading the render target failed");
			return false;
		}
		OutPixels.Reserve(Colors.Num());
		for (const FColor& C : Colors)
		{
			OutPixels.Add(FromBytes(C));
		}
		return true;
	}
	if (!Resource->ReadLinearColorPixels(OutPixels))
	{
		OutError = TEXT("reading the render target failed");
		return false;
	}
	return true;
}

FPyTensor UTensorFlowBlueprintLibrary::PixelsToTensor(const TArray<FLinearColor>& Pixels, int32 Width, int32 Height, const FTensorFlowImageOptions& Options)
{
	using namespace TensorFlowImagePrivate;
	const int32 OutW = Options.Size.X > 0 ? Options.Size.X : Width;
	const int32 OutH = Options.Size.Y > 0 ? Options.Size.Y : Height;
	const int32 C = Options.Channels == ETensorFlowChannels::Grayscale ? 1 : Options.Channels == ETensorFlowChannels::RGB ? 3 : 4;
	TArray<float> Values;
	if (Pixels.Num() != Width * Height || OutW <= 0 || OutH <= 0)
	{
		return FPyTensor();
	}
	Values.SetNumUninitialized(OutW * OutH * C);
	const bool bResize = OutW != Width || OutH != Height;
	for (int32 Y = 0; Y < OutH; ++Y)
	{
		for (int32 X = 0; X < OutW; ++X)
		{
			//Pixel centers map onto the source
			const FLinearColor P = bResize ? Sample(Pixels, Width, Height, (X + 0.5f) * Width / OutW - 0.5f, (Y + 0.5f) * Height / OutH - 0.5f) : Pixels[Y * Width + X];
			float Ch[4] = { P.R, P.G, P.B, P.A };
			if (C == 1)
			{
				Ch[0] = (P.R + P.G + P.B) / 3.f;
			}
			for (int32 k = 0; k < C; ++k)
			{
				float V = Ch[k];
				if (Options.bInvert && k < 3)
				{
					V = 1.f - V;
				}
				V = Options.Normalization == ETensorFlowNormalization::MinusOneToOne ? V * 2.f - 1.f : Options.Normalization == ETensorFlowNormalization::Raw ? V * 255.f : V;
				const int32 Index = Options.bChannelsFirst ? (k * OutH + Y) * OutW + X : (Y * OutW + X) * C + k;
				Values[Index] = V;
			}
		}
	}
	return FPyTensor::FromFloats(Values, Options.bChannelsFirst ? TArray<int64>{ C, OutH, OutW } : TArray<int64>{ OutH, OutW, C });
}

FPyTensor UTensorFlowBlueprintLibrary::TextureToTensor(UTexture2D* Texture, const FTensorFlowImageOptions& Options)
{
	TArray<FLinearColor> Pixels;
	int32 W = 0, H = 0;
	FString Error;
	if (!ReadTexture(Texture, Pixels, W, H, Error))
	{
		UE_LOG(LogTensorFlow, Warning, TEXT("TextureToTensor: %s"), *Error);
		return FPyTensor();
	}
	return PixelsToTensor(Pixels, W, H, Options);
}

FPyTensor UTensorFlowBlueprintLibrary::RenderTargetToTensor(UTextureRenderTarget2D* RenderTarget, const FTensorFlowImageOptions& Options)
{
	TArray<FLinearColor> Pixels;
	int32 W = 0, H = 0;
	FString Error;
	if (!ReadRenderTarget(RenderTarget, Pixels, W, H, Error))
	{
		UE_LOG(LogTensorFlow, Warning, TEXT("RenderTargetToTensor: %s"), *Error);
		return FPyTensor();
	}
	return PixelsToTensor(Pixels, W, H, Options);
}

UTexture2D* UTensorFlowBlueprintLibrary::TensorToTexture(const FPyTensor& Tensor, bool bChannelsFirst)
{
	using namespace TensorFlowImagePrivate;
	if (!Tensor.IsValid() || Tensor.Shape.Num() < 2 || Tensor.Shape.Num() > 3)
	{
		UE_LOG(LogTensorFlow, Warning, TEXT("TensorToTexture: expected a (H, W), (H, W, C) or (C, H, W) tensor"));
		return nullptr;
	}
	int32 H = 0, W = 0, C = 1;
	if (Tensor.Shape.Num() == 2)
	{
		H = (int32)Tensor.Shape[0];
		W = (int32)Tensor.Shape[1];
	}
	else if (bChannelsFirst)
	{
		C = (int32)Tensor.Shape[0];
		H = (int32)Tensor.Shape[1];
		W = (int32)Tensor.Shape[2];
	}
	else
	{
		H = (int32)Tensor.Shape[0];
		W = (int32)Tensor.Shape[1];
		C = (int32)Tensor.Shape[2];
	}
	if (C != 1 && C != 3 && C != 4)
	{
		UE_LOG(LogTensorFlow, Warning, TEXT("TensorToTexture: %d channels (1, 3 or 4 expected)"), C);
		return nullptr;
	}
	const TArray<float> Values = Tensor.ToFloats();
	const float Scale = Tensor.DType == EPyDType::UInt8 ? 1.f / 255.f : 1.f;
	auto At = [&](int32 Pixel, int32 k) -> float
	{
		const int32 Y = Pixel / W, X = Pixel % W;
		const int32 Index = (C == 1 || !bChannelsFirst) ? Pixel * C + k : (k * H + Y) * W + X;
		return Values[Index] * Scale;
	};
	return MakeTexture(W, H, [&](int32 i)
	{
		if (C == 1)
		{
			const uint8 V = ToByte(At(i, 0));
			return FColor(V, V, V, 255);
		}
		return FColor(ToByte(At(i, 0)), ToByte(At(i, 1)), ToByte(At(i, 2)), C == 4 ? ToByte(At(i, 3)) : 255);
	});
}

TArray<float> UTensorFlowBlueprintLibrary::Conv_GreyScaleTexture2DToFloatArray(UTexture2D* InTexture)
{
	FTensorFlowImageOptions Options;
	Options.Channels = ETensorFlowChannels::Grayscale;
	return TextureToTensor(InTexture, Options).ToFloats();
}

TArray<float> UTensorFlowBlueprintLibrary::Conv_Texture2DToFloatArray(UTexture2D* InTexture)
{
	FTensorFlowImageOptions Options;
	Options.Channels = ETensorFlowChannels::RGBA;
	return TextureToTensor(InTexture, Options).ToFloats();
}

TArray<float> UTensorFlowBlueprintLibrary::InvertFloatArray(const TArray<float>& InFloatArray)
{
	TArray<float> Out;
	Out.Reserve(InFloatArray.Num());
	for (const float V : InFloatArray)
	{
		Out.Add(1.f - V);
	}
	return Out;
}

UTexture2D* UTensorFlowBlueprintLibrary::Conv_GrayScaleFloatArrayToTexture2D(const TArray<float>& InFloatArray, const FVector2D Size)
{
	using namespace TensorFlowImagePrivate;
	int32 W = (int32)Size.X, H = (int32)Size.Y;
	if (W <= 0 || H <= 0)
	{
		W = H = SquareSide(InFloatArray.Num());
	}
	if (W <= 0 || W * H != InFloatArray.Num())
	{
		UE_LOG(LogTensorFlow, Warning, TEXT("ToTexture2D (Grayscale Array): %d values don't make a %dx%d image (give Size, or a square count)"), InFloatArray.Num(), W, H);
		return nullptr;
	}
	return MakeTexture(W, H, [&](int32 i) { const uint8 V = ToByte(InFloatArray[i]); return FColor(V, V, V, 255); });
}

UTexture2D* UTensorFlowBlueprintLibrary::Conv_FloatArrayToTexture2D(const TArray<float>& InFloatArray, const FVector2D Size)
{
	using namespace TensorFlowImagePrivate;
	int32 W = (int32)Size.X, H = (int32)Size.Y;
	if (W <= 0 || H <= 0)
	{
		W = H = InFloatArray.Num() % 4 == 0 ? SquareSide(InFloatArray.Num() / 4) : 0;
	}
	if (W <= 0 || W * H * 4 != InFloatArray.Num())
	{
		UE_LOG(LogTensorFlow, Warning, TEXT("ToTexture2D (Float Array): %d values don't make a %dx%d RGBA image (4 per pixel)"), InFloatArray.Num(), W, H);
		return nullptr;
	}
	return MakeTexture(W, H, [&](int32 i)
	{
		return FColor(ToByte(InFloatArray[i * 4]), ToByte(InFloatArray[i * 4 + 1]), ToByte(InFloatArray[i * 4 + 2]), ToByte(InFloatArray[i * 4 + 3]));
	});
}

UTexture2D* UTensorFlowBlueprintLibrary::Conv_RenderTargetTextureToTexture2D(UTextureRenderTarget2D* InTexture)
{
	using namespace TensorFlowImagePrivate;
	TArray<FLinearColor> Pixels;
	int32 W = 0, H = 0;
	FString Error;
	if (!ReadRenderTarget(InTexture, Pixels, W, H, Error))
	{
		UE_LOG(LogTensorFlow, Warning, TEXT("ToTexture2D (Render Target 2D): %s"), *Error);
		return nullptr;
	}
	return MakeTexture(W, H, [&](int32 i) { const FLinearColor& P = Pixels[i]; return FColor(ToByte(P.R), ToByte(P.G), ToByte(P.B), ToByte(P.A)); });
}

TArray<float> UTensorFlowBlueprintLibrary::Conv_ByteToFloatArray(const TArray<uint8>& InByteArray, float Scale)
{
	TArray<float> Out;
	Out.Reserve(InByteArray.Num());
	for (const uint8 B : InByteArray)
	{
		Out.Add(B * Scale);
	}
	return Out;
}
