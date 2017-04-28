#pragma once

#include "AudioCaptureData.generated.h"

USTRUCT(BlueprintType)
struct AUDIOCAPTURE_API FAudioCaptureOptions
{
	GENERATED_USTRUCT_BODY()
	FAudioCaptureOptions();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Capture Options")
	int32 SampleRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Capture Options")
	int32 BitsPerSample;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Capture Options")
	int32 Channels;
};