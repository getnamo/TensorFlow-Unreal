#include "AudioCapturePrivatePCH.h"
#include "IAudioCapture.h"
#include "AudioCaptureComponent.h"

UAudioCaptureComponent::UAudioCaptureComponent(const FObjectInitializer &init) : UActorComponent(init)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	bDidStartCapture = false;
}

void UAudioCaptureComponent::StartCapture()
{
	if (IAudioCapture::IsAvailable())
	{
		IAudioCapture::Get().SetOptions(Options);
		IAudioCapture::Get().StartCapture();
		bDidStartCapture = true;
	}
}

void UAudioCaptureComponent::StopCapture()
{
	if (IAudioCapture::IsAvailable())
	{
		IAudioCapture::Get().StopCapture();
	}
}

void UAudioCaptureComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (IAudioCapture::IsAvailable())
	{
		IAudioCapture::Get().AddAudioComponent(this);
	}
}

void UAudioCaptureComponent::UninitializeComponent()
{
	if (IAudioCapture::IsAvailable())
	{
		IAudioCapture::Get().RemoveAudioComponent(this);

		//auto-stop recordings we started.
		if (bDidStartCapture)
		{
			StopCapture();
		}
	}

	Super::UninitializeComponent();
}
