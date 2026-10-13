#include "TFAudioCaptureComponent.h"
#include "Async/Async.h"
#include "AudioCaptureCore.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, TFAudioCapture);

/** Converts device audio to the component's format: channels, linear resampling across chunks, PCM. */
struct UTFAudioCaptureComponent::FState
{
	FCriticalSection Lock;
	TWeakObjectPtr<UTFAudioCaptureComponent> Owner;
	FAudioCaptureOptions Options;
	TArray<uint8> All;
	float AllPeak = 0.f;
	double Position = 0.0; //Of the next output sample, in input frames relative to the current chunk
	TArray<float> Previous; //Last input frame of the previous chunk, per output channel
	int32 InputRate = 0;

	void Process(const float* In, int32 Frames, int32 InChannels, int32 InRate)
	{
		if (!In || Frames <= 0 || InChannels <= 0 || InRate <= 0)
		{
			return;
		}
		FScopeLock L(&Lock);
		const int32 OutChannels = FMath::Clamp(Options.Channels, 1, 2);
		const int32 OutRate = FMath::Max(Options.SampleRate, 1000);
		const int32 Bits = Options.BitsPerSample == 8 ? 8 : 16;

		//Channels first: mono average, or the first two (a mono device is duplicated)
		TArray<float> Mixed;
		Mixed.SetNumUninitialized(Frames * OutChannels);
		for (int32 F = 0; F < Frames; ++F)
		{
			const float* Frame = In + (int64)F * InChannels;
			if (OutChannels == 1)
			{
				float Sum = 0.f;
				for (int32 C = 0; C < InChannels; ++C)
				{
					Sum += Frame[C];
				}
				Mixed[F] = Sum / InChannels;
			}
			else
			{
				Mixed[F * 2] = Frame[0];
				Mixed[F * 2 + 1] = Frame[FMath::Min(1, InChannels - 1)];
			}
		}
		if (InRate != InputRate || Previous.Num() != OutChannels)
		{
			InputRate = InRate;
			Position = 0.0;
			Previous.SetNumZeroed(OutChannels);
			for (int32 C = 0; C < OutChannels; ++C)
			{
				Previous[C] = Mixed[C];
			}
		}

		//Linear resampling; Position may sit between the previous chunk's last frame (-1) and frame 0
		const double Step = (double)InRate / OutRate;
		TArray<uint8> Bytes;
		float Peak = 0.f;
		while (Position < Frames - 1)
		{
			const int32 I0 = FMath::FloorToInt(Position);
			const float Frac = (float)(Position - I0);
			for (int32 C = 0; C < OutChannels; ++C)
			{
				const float A = I0 < 0 ? Previous[C] : Mixed[I0 * OutChannels + C];
				const float B = Mixed[(I0 + 1) * OutChannels + C];
				const float V = FMath::Clamp(A + (B - A) * Frac, -1.f, 1.f);
				Peak = FMath::Max(Peak, FMath::Abs(V));
				if (Bits == 8)
				{
					Bytes.Add((uint8)FMath::Clamp(FMath::RoundToInt(V * 127.f) + 128, 0, 255));
				}
				else
				{
					const int16 S = (int16)FMath::RoundToInt(V * 32767.f);
					Bytes.Add((uint8)(S & 0xFF));
					Bytes.Add((uint8)((S >> 8) & 0xFF));
				}
			}
			Position += Step;
		}
		Position -= Frames;
		for (int32 C = 0; C < OutChannels; ++C)
		{
			Previous[C] = Mixed[(Frames - 1) * OutChannels + C];
		}
		if (Bytes.Num() == 0)
		{
			return;
		}
		All.Append(Bytes);
		AllPeak = FMath::Max(AllPeak, Peak);
		AsyncTask(ENamedThreads::GameThread, [WeakOwner = Owner, Bytes = MoveTemp(Bytes), Peak]()
		{
			if (UTFAudioCaptureComponent* Self = WeakOwner.Get())
			{
				Self->OnAudioData.Broadcast(Bytes, Peak);
			}
		});
	}
};

/** The default capture device, feeding a component's state. */
class FTFMicrophone
{
public:
	bool Open(TSharedPtr<UTFAudioCaptureComponent::FState, ESPMode::ThreadSafe> Target);
	void Close();
	~FTFMicrophone() { Close(); }

private:
	Audio::FAudioCapture Capture;
	bool bOpen = false;
};

UTFAudioCaptureComponent::UTFAudioCaptureComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTFAudioCaptureComponent::StartCapture()
{
	if (IsCapturing())
	{
		return;
	}
	State = MakeShared<FState, ESPMode::ThreadSafe>();
	State->Owner = this;
	State->Options = Options;
	Microphone = MakeShared<FTFMicrophone, ESPMode::ThreadSafe>();
	if (!Microphone->Open(State))
	{
		UE_LOG(LogTemp, Warning, TEXT("TFAudioCapture: could not open the microphone (is the AudioCapture plugin enabled and a device connected?)"));
		Microphone.Reset();
	}
}

void UTFAudioCaptureComponent::StopCapture()
{
	if (Microphone.IsValid())
	{
		Microphone->Close(); //Joins the capture callback: nothing touches State after this
		Microphone.Reset();
	}
	if (State.IsValid())
	{
		TArray<uint8> All;
		float Peak = 0.f;
		{
			FScopeLock L(&State->Lock);
			All = MoveTemp(State->All);
			Peak = State->AllPeak;
		}
		State.Reset();
		OnCaptureFinished.Broadcast(All, Peak);
	}
}

bool UTFAudioCaptureComponent::IsCapturing() const
{
	return Microphone.IsValid();
}

void UTFAudioCaptureComponent::ProcessAudio(const float* Interleaved, int32 NumFrames, int32 NumChannels, int32 SampleRate)
{
	if (!State.IsValid())
	{
		State = MakeShared<FState, ESPMode::ThreadSafe>();
		State->Owner = this;
		State->Options = Options;
	}
	State->Process(Interleaved, NumFrames, NumChannels, SampleRate);
}

void UTFAudioCaptureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsCapturing())
	{
		StopCapture();
	}
	Super::EndPlay(EndPlayReason);
}

void UTFAudioCaptureComponent::ConvertRawToWav(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes)
{
	OutBytes = FWavHeader::RawPCMToWav(InBytes, Options);
}

void UTFAudioCaptureComponent::ConvertWavToRaw(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes, FAudioCaptureOptions& OutOptions)
{
	FWavHeader::WavToRawPCM(InBytes, OutBytes, OutOptions);
}

bool FTFMicrophone::Open(TSharedPtr<UTFAudioCaptureComponent::FState, ESPMode::ThreadSafe> Target)
{
	Audio::FAudioCaptureDeviceParams Params;
	bOpen = Capture.OpenAudioCaptureStream(Params, [Target](const void* InAudio, int32 NumFrames, int32 NumChannels, int32 SampleRate, double, bool)
	{
		Target->Process(static_cast<const float*>(InAudio), NumFrames, NumChannels, SampleRate);
	}, 1024);
	if (bOpen && !Capture.StartStream())
	{
		Capture.CloseStream();
		bOpen = false;
	}
	return bOpen;
}

void FTFMicrophone::Close()
{
	if (bOpen)
	{
		Capture.StopStream();
		Capture.CloseStream();
		bOpen = false;
	}
}
