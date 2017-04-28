#pragma once
#include "AudioCaptureData.h"

class IAudioCaptureInterface
{
public:
	/** Required API */
	
	/** Start capturing audio, whenever there is audio data, call passed-in lambda */
	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData = nullptr, TFunction<void(const TArray<uint8>&)> OnCaptureFinished = nullptr) {};

	/** Stop the audio capture and cleanup */
	virtual void StopCapture() {};

	virtual void SetOptions(const FAudioCaptureOptions& InOptions) {};
};