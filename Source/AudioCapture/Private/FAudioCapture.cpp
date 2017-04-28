#include "AudioCapturePrivatePCH.h"
#include "IAudioCapture.h"
#include "FWindowsAudioCapture.h"

class FAudioCapture : public IAudioCapture
{
public:

	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData) override;
	virtual void StopCapture() override;

private:
	TSharedPtr<FWindowsAudioCapture> WindowsCapture;
};

void FAudioCapture::StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioBufferReceived)
{
	if (!WindowsCapture.IsValid())
	{
		WindowsCapture = MakeShareable(new FWindowsAudioCapture);
	}

	WindowsCapture->StartCapture(OnAudioBufferReceived);
}

void FAudioCapture::StopCapture()
{
	WindowsCapture->StopCapture();
}


IMPLEMENT_MODULE(FAudioCapture, AudioCapture)