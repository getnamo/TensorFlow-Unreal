#pragma once

#include "IAudioCaptureInterface.h"

class FWindowsAudioCapture : public IAudioCaptureInterface
{
public:
	FWindowsAudioCapture();

	virtual void StartCapture(TFunction<void(const TArray<uint8>&, float)> OnAudioData = nullptr, TFunction<void(const TArray<uint8>&, float)> OnCaptureFinished = nullptr) override;
	virtual void StopCapture() override;
	virtual void SetOptions(const FAudioCaptureOptions& InOptions) override;

	FAudioCaptureOptions Options;
private:

	float CalculateMaxAudioLevel(TArray<uint8>& Buffer, int32 BitsPerSample);
	FThreadSafeBool bRunLoopActive;
};