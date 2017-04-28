#pragma once

#include "IAudioCaptureInterface.h"

class FWindowsAudioCapture : public IAudioCaptureInterface
{
public:

	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData) override;
	virtual void StopCapture() override;

private:
	FThreadSafeBool bRunLoopActive;
};