#include "AudioCapturePrivatePCH.h"
#include "IAudioCapture.h"
#include "AudioCaptureComponent.h"

UAudioCaptureComponent::UAudioCaptureComponent(const FObjectInitializer &init) : UActorComponent(init)
{
}

void UAudioCaptureComponent::StartRecording()
{
	if (IAudioCapture::IsAvailable())
	{
		IAudioCapture::Get().StartCapture(nullptr);
	}
}

void UAudioCaptureComponent::StopRecording()
{
	if (IAudioCapture::IsAvailable())
	{
		IAudioCapture::Get().StopCapture();
	}
}

void UAudioCaptureComponent::InitializeComponent()
{
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
	}
}
