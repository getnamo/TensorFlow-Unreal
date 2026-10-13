#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioCaptureData.h"
#include "TFAudioCaptureComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAudioDataSignature, const TArray<uint8>&, Bytes, float, MaxLevel);

/**
 * Microphone capture as raw PCM bytes in the format of Options (16 kHz 16-bit mono by default), for
 * TensorFlow-Unreal 1.x projects. New projects: a Python Audio Stream sends float32 audio straight to a
 * Python script.
 */
UCLASS(ClassGroup = "Sound", meta = (BlueprintSpawnableComponent))
class TFAUDIOCAPTURE_API UTFAudioCaptureComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UTFAudioCaptureComponent();

	/** New audio as it streams in (game thread): PCM bytes in Options' format and the chunk's peak level (0..1). */
	UPROPERTY(BlueprintAssignable, Category = "Audio Capture Events")
	FAudioDataSignature OnAudioData;

	/** StopCapture: everything captured since StartCapture, and its peak level. */
	UPROPERTY(BlueprintAssignable, Category = "Audio Capture Events")
	FAudioDataSignature OnCaptureFinished;

	/** Output format: SampleRate (resampled from the device), Channels (1 or 2), BitsPerSample (8 or 16). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Capture Options")
	FAudioCaptureOptions Options;

	UFUNCTION(BlueprintCallable, Category = "Utilities|Audio Capture")
	void StartCapture();

	UFUNCTION(BlueprintCallable, Category = "Utilities|Audio Capture")
	void StopCapture();

	UFUNCTION(BlueprintPure, Category = "Utilities|Audio Capture")
	bool IsCapturing() const;

	/** Raw PCM (in Options' format) -> WAV file bytes. */
	UFUNCTION(BlueprintCallable, Category = "Utilities|Audio Capture")
	void ConvertRawToWav(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes);

	/** WAV file bytes -> raw PCM, with the WAV's format in OutOptions. */
	UFUNCTION(BlueprintCallable, Category = "Utilities|Audio Capture")
	void ConvertWavToRaw(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes, FAudioCaptureOptions& OutOptions);

	/** C++, any thread: convert device audio (interleaved float) to Options' format and emit it, as the
	 microphone callback does. */
	void ProcessAudio(const float* Interleaved, int32 NumFrames, int32 NumChannels, int32 SampleRate);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Internal: the format conversion, shared with the capture thread. */
	struct FState;

private:
	TSharedPtr<FState, ESPMode::ThreadSafe> State;
	TSharedPtr<class FTFMicrophone, ESPMode::ThreadSafe> Microphone;
};
