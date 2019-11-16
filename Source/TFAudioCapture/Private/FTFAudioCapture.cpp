#include "ITFAudioCapture.h"
#include "TFAudioCaptureComponent.h"
#include "CULambdaRunnable.h"
#include "FWindowsAudioCapture.h"

class FTFAudioCapture : public ITFAudioCapture
{
public:

	virtual void StartCapture(TFunction<void(const TArray<uint8>&, float)> OnAudioData = nullptr, TFunction<void(const TArray<uint8>&, float)> OnCaptureFinished = nullptr) override;
	virtual void StopCapture() override;
	virtual void SetOptions(const FAudioCaptureOptions& Options) override;

	virtual void AddAudioComponent(const UTFAudioCaptureComponent* Component) override;
	virtual void RemoveAudioComponent(const UTFAudioCaptureComponent* Component) override;
	virtual void AddAudioDelegate(IAudioDataInterface* Delegate) override;
	virtual void RemoveAudioDelegate(IAudioDataInterface* Delegate) override;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FWindowsAudioCapture> WindowsCapture;
	TArray<IAudioDataInterface*> Delegates;
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

void FTFAudioCapture::StartCapture(TFunction<void(const TArray<uint8>&, float)> OnAudioData, TFunction<void(const TArray<uint8>&, float)> OnCaptureFinished)
{
	TFunction<void(const TArray<uint8>&, float)> OnDataDelegate = [this, OnAudioData] (const TArray<uint8>& AudioData, float AudioMaxLevel)
	{
		//Call each added component function inside game thread
		FCULambdaRunnable::RunShortLambdaOnGameThread([this, AudioData, OnAudioData, AudioMaxLevel]
		{
			for (auto Delegate : Delegates)
			{
				Delegate->OnAudioDataEvent(AudioData, AudioMaxLevel);
			}

			//Also if valid pass it to the new delegate
			if (OnAudioData != nullptr)
			{
				OnAudioData(AudioData, AudioMaxLevel);
			}
		});
	};

	TFunction<void(const TArray<uint8>&, float)> OnFinishedDelegate = [this, OnCaptureFinished](const TArray<uint8>& AudioData, float AudioMaxLevel)
	{
		//Call each added component function inside game thread
		FCULambdaRunnable::RunShortLambdaOnGameThread([this, AudioData, OnCaptureFinished, AudioMaxLevel]
		{
			for (auto Delegate : Delegates)
			{
				Delegate->OnCaptureFinishedEvent(AudioData, AudioMaxLevel);
			}

			//Also if valid pass it to the new delegate
			if (OnCaptureFinished != nullptr)
			{
				OnCaptureFinished(AudioData, AudioMaxLevel);
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
	AddAudioDelegate((IAudioDataInterface*)Component);
}

void FTFAudioCapture::RemoveAudioComponent(const UTFAudioCaptureComponent* Component)
{
	RemoveAudioDelegate((IAudioDataInterface*)Component);
}

void FTFAudioCapture::AddAudioDelegate(IAudioDataInterface* Delegate)
{
	Delegates.Add(Delegate);
}

void FTFAudioCapture::RemoveAudioDelegate(IAudioDataInterface* Delegate)
{
	Delegates.Remove(Delegate);
}


IMPLEMENT_MODULE(FTFAudioCapture, TFAudioCapture)


