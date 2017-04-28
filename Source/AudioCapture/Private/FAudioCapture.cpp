#include "AudioCapturePrivatePCH.h"
#include "IAudioCapture.h"
#include "AudioCaptureComponent.h"
#include "LambdaRunnable.h"
#include "FWindowsAudioCapture.h"

class FAudioCapture : public IAudioCapture
{
public:

	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData) override;
	virtual void StopCapture() override;
	virtual void AddAudioComponent(const UAudioCaptureComponent* Component) override;
	virtual void RemoveAudioComponent(const UAudioCaptureComponent* Component) override;

private:
	TSharedPtr<FWindowsAudioCapture> WindowsCapture;
	TArray<UAudioCaptureComponent*> Components;
};

void FAudioCapture::StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioBufferReceived)
{
	if (!WindowsCapture.IsValid())
	{
		WindowsCapture = MakeShareable(new FWindowsAudioCapture);
	}

	TFunction<void(const TArray<uint8>&)> OnDataDelegate = [this, OnAudioBufferReceived] (const TArray<uint8>& AudioData)
	{
		//Call each added component function inside gamethread
		FLambdaRunnable::RunShortLambdaOnGameThread([this, AudioData, OnAudioBufferReceived]
		{
			for (auto Component : Components)
			{
				Component->OnAudioData.Broadcast(AudioData);
			}

			//Also if valid pass it to the new delegate
			if (OnAudioBufferReceived != nullptr)
			{
				OnAudioBufferReceived(AudioData);
			}
		});
	};

	WindowsCapture->StartCapture(OnDataDelegate);
}

void FAudioCapture::StopCapture()
{
	WindowsCapture->StopCapture();
}

void FAudioCapture::AddAudioComponent(const UAudioCaptureComponent* Component)
{
	Components.Add((UAudioCaptureComponent*)Component);
}

void FAudioCapture::RemoveAudioComponent(const UAudioCaptureComponent* Component)
{
	Components.Remove((UAudioCaptureComponent*)Component);
}

IMPLEMENT_MODULE(FAudioCapture, AudioCapture)