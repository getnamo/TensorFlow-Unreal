#pragma once

#include "AudioCaptureData.generated.h"

USTRUCT(BlueprintType)
struct AUDIOCAPTURE_API FAudioCaptureOptions
{
	GENERATED_USTRUCT_BODY()
	FAudioCaptureOptions();

	UPROPERTY(BlueprintReadOnly, Category = "Audio Capture Options")
	int32 SampleRate;
};