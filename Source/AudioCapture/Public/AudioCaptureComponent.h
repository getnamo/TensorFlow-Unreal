#pragma once

#include "Components/ActorComponent.h"
#include "AudioCaptureComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudioDataSignature, const TArray<uint8>&, Bytes);

/**
*
*/
UCLASS(ClassGroup = "Sound", meta = (BlueprintSpawnableComponent))
class AUDIOCAPTURE_API UAudioCaptureComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:

	/** Event received whenever new audio data streams in. */
	UPROPERTY(BlueprintAssignable, Category = "SocketIO Events")
	FAudioDataSignature OnAudioData;

	/** Convert a UTexture2D to float array, assuming a square texture*/
	UFUNCTION(BlueprintCallable, Category = "Utilities|Audio Capture")
	void StartRecording();

	UFUNCTION(BlueprintCallable, Category = "Utilities|Audio Capture")
	void StopRecording();


protected:
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
};
