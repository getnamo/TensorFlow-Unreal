#pragma once

#include "IAudioCaptureInterface.h"

class FWindowsAudioCapture : public IAudioCaptureInterface
{
public:
	FWindowsAudioCapture();

	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData = nullptr, TFunction<void(const TArray<uint8>&)> OnCaptureFinished = nullptr) override;
	virtual void StopCapture() override;
	virtual void SetOptions(const FAudioCaptureOptions& InOptions) override;

	FAudioCaptureOptions Options;
private:
	FThreadSafeBool bRunLoopActive;
};