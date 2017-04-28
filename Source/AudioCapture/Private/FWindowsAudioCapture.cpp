#include "AudioCapturePrivatePCH.h"
#include "LambdaRunnable.h"
#include "FWindowsAudioCapture.h"

void FWindowsAudioCapture::StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData)
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

void FWindowsAudioCapture::StopCapture()
{
	bRunLoopActive = false;
}
