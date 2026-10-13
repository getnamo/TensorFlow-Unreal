#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PythonTypes.h"
#include "TensorFlowBlueprintLibrary.generated.h"

class UTexture2D;
class UTextureRenderTarget2D;

UENUM(BlueprintType)
enum class ETensorFlowChannels : uint8
{
	/** One channel: (R + G + B) / 3. */
	Grayscale,
	RGB,
	RGBA,
};

UENUM(BlueprintType)
enum class ETensorFlowNormalization : uint8
{
	/** 0..1 */
	ZeroToOne,
	/** -1..1 */
	MinusOneToOne,
	/** 0..255 */
	Raw,
};

/** How an image becomes a float32 tensor. */
USTRUCT(BlueprintType)
struct TENSORFLOW_API FTensorFlowImageOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Image")
	ETensorFlowChannels Channels = ETensorFlowChannels::RGB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Image")
	ETensorFlowNormalization Normalization = ETensorFlowNormalization::ZeroToOne;

	/** Resize (bilinear) to this size first; 0 keeps the image's size. E.g. 28 x 28 for MNIST. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Image")
	FIntPoint Size = FIntPoint::ZeroValue;

	/** 1 - value (before scaling), e.g. dark-on-light drawings for MNIST's light-on-dark digits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Image")
	bool bInvert = false;

	/** (C, H, W) instead of (H, W, C): PyTorch's layout. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TensorFlow Image")
	bool bChannelsFirst = false;
};

/**
 * Images, float arrays and tensors. Textures are read from the editor's source data, a CPU copy (texture
 * Availability: CPU), or uncompressed platform data (e.g. Compression: UserInterface2D or Grayscale);
 * render targets with a blocking read (use a Python Render Target Stream for every frame).
 */
UCLASS()
class TENSORFLOW_API UTensorFlowBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** Image -> float32 tensor ((H, W, C), or (C, H, W)) for Send Tensor Input. */
	UFUNCTION(BlueprintPure, Category = "Utilities|TensorFlow")
	static FPyTensor TextureToTensor(UTexture2D* Texture, const FTensorFlowImageOptions& Options);

	/** As TextureToTensor; reads the render target now (blocking). */
	UFUNCTION(BlueprintCallable, Category = "Utilities|TensorFlow")
	static FPyTensor RenderTargetToTensor(UTextureRenderTarget2D* RenderTarget, const FTensorFlowImageOptions& Options);

	/** A (H, W), (H, W, 1/3/4) or (C, H, W) tensor -> a transient texture. Float values are taken as 0..1,
	 uint8 as 0..255. */
	UFUNCTION(BlueprintCallable, Category = "Utilities|TensorFlow")
	static UTexture2D* TensorToTexture(const FPyTensor& Tensor, bool bChannelsFirst = false);

	// ---- 1.x functions (kept; now on the read paths above) ------------------------------------------

	/** Grayscale 0..1, one float per pixel. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToGrayScaleFloatArray (Texture2D)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static TArray<float> Conv_GreyScaleTexture2DToFloatArray(UTexture2D* InTexture);

	/** RGBA 0..1, four floats per pixel (1.x returned the texture's byte order, often BGRA). */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToFloatArray (Texture2D)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static TArray<float> Conv_Texture2DToFloatArray(UTexture2D* InTexture);

	/** 1 - value for each value. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "InvertFloatArray"), Category = "Utilities|TensorFlow")
	static TArray<float> InvertFloatArray(const TArray<float>& InFloatArray);

	/** Grayscale 0..1 values -> texture; square unless Size is given. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTexture2D (Grayscale Array)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static UTexture2D* Conv_GrayScaleFloatArrayToTexture2D(const TArray<float>& InFloatArray, const FVector2D Size = FVector2D(0, 0));

	/** RGBA 0..1 values (four per pixel) -> texture; square unless Size is given. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTexture2D (Float Array)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static UTexture2D* Conv_FloatArrayToTexture2D(const TArray<float>& InFloatArray, const FVector2D Size = FVector2D(0, 0));

	/** A transient copy of a render target (color; blocking read). */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToTexture2D (Render Target 2D)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static UTexture2D* Conv_RenderTargetTextureToTexture2D(UTextureRenderTarget2D* InTexture);

	/** Bytes -> floats, each multiplied by Scale. */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToFloatArray (bytes)", BlueprintAutocast), Category = "Utilities|TensorFlow")
	static TArray<float> Conv_ByteToFloatArray(const TArray<uint8>& InByteArray, float Scale = 1.f);

	/** C++: a texture's pixels as linear 0..1 values of its stored data (no gamma conversion). False (with
	 the reason) if the texture can't be read on the CPU. */
	static bool ReadTexture(UTexture2D* Texture, TArray<FLinearColor>& OutPixels, int32& OutWidth, int32& OutHeight, FString& OutError);
	static bool ReadRenderTarget(UTextureRenderTarget2D* RenderTarget, TArray<FLinearColor>& OutPixels, int32& OutWidth, int32& OutHeight, FString& OutError);
	static FPyTensor PixelsToTensor(const TArray<FLinearColor>& Pixels, int32 Width, int32 Height, const FTensorFlowImageOptions& Options);
};
