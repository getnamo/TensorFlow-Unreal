#pragma once

#include "IAudioCaptureInterface.h"

class FWindowsAudioCapture : public IAudioCaptureInterface
{
public:
	FWindowsAudioCapture();

	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData = nullptr, TFunction<void(const TArray<uint8>&)> OnCaptureFinished = nullptr) override;
	virtual void StopCapture() override;
	virtual void SetOptions(const FAudioCaptureOptions& SampleRate) override;

	int32 SampleRate;
private:
	FThreadSafeBool bRunLoopActive;
};