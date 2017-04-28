#include "AudioCapturePrivatePCH.h"
#include "IAudioCapture.h"
#include "AudioCaptureComponent.h"
#include "LambdaRunnable.h"
#include "FWindowsAudioCapture.h"

class FAudioCapture : public IAudioCapture
{
public:

	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData = nullptr, TFunction<void(const TArray<uint8>&)> OnCaptureFinished = nullptr) override;
	virtual void StopCapture() override;
	virtual void AddAudioComponent(const UAudioCaptureComponent* Component) override;
	virtual void RemoveAudioComponent(const UAudioCaptureComponent* Component) override;

private:
	TSharedPtr<FWindowsAudioCapture> WindowsCapture;
	TArray<UAudioCaptureComponent*> Components;
};

void FAudioCapture::StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData, TFunction<void(const TArray<uint8>&)> OnCaptureFinished)
{
	if (!WindowsCapture.IsValid())
	{
		WindowsCapture = MakeShareable(new FWindowsAudioCapture);
	}

	TFunction<void(const TArray<uint8>&)> OnDataDelegate = [this, OnAudioData] (const TArray<uint8>& AudioData)
	{
		//Call each added component function inside game thread
		FLambdaRunnable::RunShortLambdaOnGameThread([this, AudioData, OnAudioData]
		{
			for (auto Component : Components)
			{
				Component->OnAudioData.Broadcast(AudioData);
			}

			//Also if valid pass it to the new delegate
			if (OnAudioData != nullptr)
			{
				OnAudioData(AudioData);
			}
		});
	};

	TFunction<void(const TArray<uint8>&)> OnFinishedDelegate = [this, OnCaptureFinished](const TArray<uint8>& AudioData)
	{
		//Call each added component function inside game thread
		FLambdaRunnable::RunShortLambdaOnGameThread([this, AudioData, OnCaptureFinished]
		{
			for (auto Component : Components)
			{
				Component->OnCaptureFinished.Broadcast(AudioData);
			}

			//Also if valid pass it to the new delegate
			if (OnCaptureFinished != nullptr)
			{
				OnCaptureFinished(AudioData);
			}

		});
	};

	WindowsCapture->StartCapture(OnDataDelegate, OnFinishedDelegate);
}

void FAudioCapture::StopCapture()
{
	if (WindowsCapture.IsValid())
	{
		WindowsCapture->StopCapture();
	}
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