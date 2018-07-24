#pragma once

//Native interface to receive callbacks from audio capture
class IAudioDataInterface
{
public:
	/** Required API */
	
	/** When data gets streamed */
	virtual void OnAudioDataEvent(const TArray<uint8>& Bytes, float MaxLevel) {};

	/** When capture is complete */
	virtual void OnCaptureFinishedEvent(const TArray<uint8>& Bytes, float MaxLevel) {};
};