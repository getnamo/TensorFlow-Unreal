#pragma once

#include "Components/ActorComponent.h"
#include "AudioCaptureData.h"
#include "AudioCaptureComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudioDataSignature, const TArray<uint8>&, Bytes);

/**
* Component used to capture microphone audio and emit bytes as the data streams in
*/
UCLASS(ClassGroup = "Sound", meta = (BlueprintSpawnableComponent))
class AUDIOCAPTURE_API UAudioCaptureComponent : public UActorComponent
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utilities|Audio Capture")
	FAudioCaptureOptions Options;

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

private:
	bool bDidStartCapture;
};
