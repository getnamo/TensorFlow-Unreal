#include "TensorFlowComponent.h"
#include "TensorFlow.h"
#include "Dom/JsonObject.h"

void UTensorFlowComponent::SendTextureInput(UTexture2D* Texture, const FTensorFlowImageOptions& Options, const FString& FunctionName)
{
	TArray<FLinearColor> Pixels;
	int32 W = 0, H = 0;
	FString Error;
	if (!UTensorFlowBlueprintLibrary::ReadTexture(Texture, Pixels, W, H, Error))
	{
		UE_LOG(LogTensorFlow, Error, TEXT("SendTextureInput: %s"), *Error);
		OnScriptError.Broadcast(Error, FString());
		return;
	}
	SendTensorInput(UTensorFlowBlueprintLibrary::PixelsToTensor(Pixels, W, H, Options), FunctionName);
}

void UTensorFlowComponent::SendRenderTargetInput(UTextureRenderTarget2D* RenderTarget, const FTensorFlowImageOptions& Options, const FString& FunctionName)
{
	TArray<FLinearColor> Pixels;
	int32 W = 0, H = 0;
	FString Error;
	if (!UTensorFlowBlueprintLibrary::ReadRenderTarget(RenderTarget, Pixels, W, H, Error))
	{
		UE_LOG(LogTensorFlow, Error, TEXT("SendRenderTargetInput: %s"), *Error);
		OnScriptError.Broadcast(Error, FString());
		return;
	}
	SendTensorInput(UTensorFlowBlueprintLibrary::PixelsToTensor(Pixels, W, H, Options), FunctionName);
}

void UTensorFlowComponent::SendJsonInputTexture(UTexture2D* Texture, const FString& FunctionName)
{
	TArray<FLinearColor> Pixels;
	int32 W = 0, H = 0;
	FString Error;
	if (!UTensorFlowBlueprintLibrary::ReadTexture(Texture, Pixels, W, H, Error))
	{
		UE_LOG(LogTensorFlow, Error, TEXT("SendJsonInputTexture: %s"), *Error);
		OnScriptError.Broadcast(Error, FString());
		return;
	}
	TArray<TSharedPtr<FJsonValue>> Gray;
	Gray.Reserve(Pixels.Num());
	for (const FLinearColor& P : Pixels)
	{
		Gray.Add(MakeShared<FJsonValueNumber>((P.R + P.G + P.B) / 3.f));
	}
	TSharedRef<FJsonObject> Size = MakeShared<FJsonObject>();
	Size->SetNumberField(TEXT("x"), W);
	Size->SetNumberField(TEXT("y"), H);
	TSharedRef<FJsonObject> Input = MakeShared<FJsonObject>();
	Input->SetArrayField(TEXT("pixels"), Gray);
	Input->SetObjectField(TEXT("size"), Size);
	SendJsonInput(PythonWire::ToJsonString(MakeShared<FJsonValueObject>(Input)), FunctionName);
}
