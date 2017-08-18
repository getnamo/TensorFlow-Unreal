#include "AudioCapturePrivatePCH.h"
#include "ITFAudioCapture.h"
#include "TFAudioCaptureComponent.h"
#include "LambdaRunnable.h"
#include "FWindowsAudioCapture.h"

class FTFAudioCapture : public ITFAudioCapture
{
public:

	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData = nullptr, TFunction<void(const TArray<uint8>&)> OnCaptureFinished = nullptr) override;
	virtual void StopCapture() override;
	virtual void SetOptions(const FAudioCaptureOptions& Options) override;

	virtual void AddAudioComponent(const UTFAudioCaptureComponent* Component) override;
	virtual void RemoveAudioComponent(const UTFAudioCaptureComponent* Component) override;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FWindowsAudioCapture> WindowsCapture;
	TArray<UTFAudioCaptureComponent*> Components;
};

void FTFAudioCapture::StartupModule()
{
	if (!WindowsCapture.IsValid())
	{
		WindowsCapture = MakeShareable(new FWindowsAudioCapture);
	}
}

void FTFAudioCapture::ShutdownModule()
{

}

void FTFAudioCapture::StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData, TFunction<void(const TArray<uint8>&)> OnCaptureFinished)
{
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

void FTFAudioCapture::StopCapture()
{
	if (WindowsCapture.IsValid())
	{
		WindowsCapture->StopCapture();
	}
}

void FTFAudioCapture::SetOptions(const FAudioCaptureOptions& Options)
{
	WindowsCapture->SetOptions(Options);
}

void FTFAudioCapture::AddAudioComponent(const UTFAudioCaptureComponent* Component)
{
	Components.Add((UTFAudioCaptureComponent*)Component);
}

void FTFAudioCapture::RemoveAudioComponent(const UTFAudioCaptureComponent* Component)
{
	Components.Remove((UTFAudioCaptureComponent*)Component);
}

IMPLEMENT_MODULE(FTFAudioCapture, TFAudioCapture)


