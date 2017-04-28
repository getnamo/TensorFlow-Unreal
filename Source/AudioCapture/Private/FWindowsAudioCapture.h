#pragma once

#include "IAudioCaptureInterface.h"

class FWindowsAudioCapture : public IAudioCaptureInterface
{
public:
	FWindowsAudioCapture();

	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData) override;
	virtual void StopCapture() override;

	int32 SampleRate;
private:
	FThreadSafeBool bRunLoopActive;
};