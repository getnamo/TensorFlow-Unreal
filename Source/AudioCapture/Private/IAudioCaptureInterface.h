#pragma once

class IAudioCaptureInterface
{
public:
	/** Required API */
	
	/** Start capturing audio, whenever there is audio data, call passed-in lambda */
	virtual void StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData) {};

	/** Stop the audio capture and cleanup */
	virtual void StopCapture() {};
};