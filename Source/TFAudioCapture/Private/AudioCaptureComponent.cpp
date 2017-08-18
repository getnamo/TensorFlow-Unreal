#include "AudioCapturePrivatePCH.h"
#include "ITFAudioCapture.h"
#include "AudioCaptureComponent.h"

UAudioCaptureComponent::UAudioCaptureComponent(const FObjectInitializer &init) : UActorComponent(init)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	bDidStartCapture = false;
}

void UAudioCaptureComponent::StartCapture()
{
	if (ITFAudioCapture::IsAvailable())
	{
		ITFAudioCapture::Get().SetOptions(Options);
		ITFAudioCapture::Get().StartCapture();
		bDidStartCapture = true;
	}
}

void UAudioCaptureComponent::StopCapture()
{
	if (ITFAudioCapture::IsAvailable())
	{
		ITFAudioCapture::Get().StopCapture();
	}
}

void UAudioCaptureComponent::ConvertRawToWav(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes)
{	
	OutBytes = FWavHeader::RawPCMToWav(InBytes, Options);
}

void UAudioCaptureComponent::ConvertWavToRaw(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes, FAudioCaptureOptions& OutOptions)
{
	FWavHeader::WavToRawPCM(InBytes, OutBytes, OutOptions);
}

void UAudioCaptureComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (ITFAudioCapture::IsAvailable())
	{
		ITFAudioCapture::Get().AddAudioComponent(this);
	}
}

void UAudioCaptureComponent::UninitializeComponent()
{
	if (ITFAudioCapture::IsAvailable())
	{
		ITFAudioCapture::Get().RemoveAudioComponent(this);

		//auto-stop recordings we started.
		if (bDidStartCapture)
		{
			StopCapture();
		}
	}

	Super::UninitializeComponent();
}
