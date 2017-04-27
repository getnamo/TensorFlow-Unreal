#pragma once

#include "AudioCapturePrivatePCH.h"
#include "IAudioCapture.h"
#include "LambdaRunnable.h"

class FAudioCapture : public IAudioCapture
{
public:

	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData) override;
	virtual void StopCapture() override;

private:
	FThreadSafeBool bRunLoopActive;
};

void FAudioCapture::StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioBufferReceived)
{
	//Only attempt to start capture once. If it's active return.
	//TODO: add multi-cast delegation?
	if (bRunLoopActive)
	{
		return;
	}

	bRunLoopActive = true;
	FThreadSafeBool* bShouldRunPtr = &bRunLoopActive;
	FLambdaRunnable::RunLambdaOnBackGroundThread([this, bShouldRunPtr]()
	{
		//Todo: fill this out with windows code
	});
}

void FAudioCapture::StopCapture()
{
	bRunLoopActive = false;
}


IMPLEMENT_MODULE(FAudioCapture, AudioCapture)