#include "TFAudioCaptureComponent.h"
#include "ITFAudioCapture.h"

UTFAudioCaptureComponent::UTFAudioCaptureComponent(const FObjectInitializer &init) : UActorComponent(init)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	bDidStartCapture = false;
}

void UTFAudioCaptureComponent::StartCapture()
{
	if (ITFAudioCapture::IsAvailable())
	{
		ITFAudioCapture::Get().SetOptions(Options);
		ITFAudioCapture::Get().StartCapture();
		bDidStartCapture = true;
	}
}

void UTFAudioCaptureComponent::StopCapture()
{
	if (ITFAudioCapture::IsAvailable())
	{
		ITFAudioCapture::Get().StopCapture();
	}
}

void UTFAudioCaptureComponent::ConvertRawToWav(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes)
{	
	OutBytes = FWavHeader::RawPCMToWav(InBytes, Options);
}

void UTFAudioCaptureComponent::ConvertWavToRaw(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes, FAudioCaptureOptions& OutOptions)
{
	FWavHeader::WavToRawPCM(InBytes, OutBytes, OutOptions);
}

void UTFAudioCaptureComponent::InitializeComponent()
{
	Super::InitializeComponent();

	UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);

	if (World->IsGameWorld() && ITFAudioCapture::IsAvailable())
	{
		
		ITFAudioCapture::Get().AddAudioComponent(this);
	}
}

void UTFAudioCaptureComponent::UninitializeComponent()
{
	UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);

	if (World->IsGameWorld() && ITFAudioCapture::IsAvailable())
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

void UTFAudioCaptureComponent::OnAudioDataEvent(const TArray<uint8>& Bytes, float MaxLevel)
{
	OnAudioData.Broadcast(Bytes, MaxLevel);
}
void UTFAudioCaptureComponent::OnCaptureFinishedEvent(const TArray<uint8>& Bytes, float MaxLevel)
{
	OnCaptureFinished.Broadcast(Bytes, MaxLevel);
}