#pragma once

#include "Components/ActorComponent.h"
#include "AudioCaptureData.h"
#include "IAudioDataInterface.h"
#include "TFAudioCaptureComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAudioDataSignature, const TArray<uint8>&, Bytes, float, MaxLevel);

/**
* Component used to capture microphone audio and emit bytes as the data streams in
*/
UCLASS(ClassGroup = "Sound", meta = (BlueprintSpawnableComponent))
class TFAUDIOCAPTURE_API UTFAudioCaptureComponent : public UActorComponent, public IAudioDataInterface
{
	GENERATED_UCLASS_BODY()

public:

	/** Event received whenever new audio data streams in. */
	UPROPERTY(BlueprintAssignable, Category = "SocketIO Events")
	FAudioDataSignature OnAudioData;

	/** Event received when the audio capture has finished (via stop) */
	UPROPERTY(BlueprintAssignable, Category = "SocketIO Events")
	FAudioDataSignature OnCaptureFinished;

	/** Start capturing audio and emitting data events */
	UFUNCTION(BlueprintCallable, Category = "Utilities|Audio Capture")
	void StartCapture();

	/** Stop capturing audio and emitting data events */
	UFUNCTION(BlueprintCallable, Category = "Utilities|Audio Capture")
	void StopCapture();

	/** Set Capturing options */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Capture Options")
	FAudioCaptureOptions Options;

	/** Convenience function to convert raw audio into wav */
	UFUNCTION(BlueprintCallable, Category = "Utilities|Audio Capture")
	void ConvertRawToWav(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes);

	/** Convenience function to convert wav into a raw PCM buffer with options containing wav format data */
	UFUNCTION(BlueprintCallable, Category = "Utilities|Audio Capture")
	void ConvertWavToRaw(const TArray<uint8>& InBytes, TArray<uint8>& OutBytes, FAudioCaptureOptions& OutOptions);

	//Start IAudioDataInterface
	virtual void OnAudioDataEvent(const TArray<uint8>& Bytes, float MaxLevel) override;
	virtual void OnCaptureFinishedEvent(const TArray<uint8>& Bytes, float MaxLevel) override;
	//End IAudioDataInterface


	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

private:
	bool bDidStartCapture;
};
