#include "AudioCapturePrivatePCH.h"
#include "AudioCaptureData.h"

FAudioCaptureOptions::FAudioCaptureOptions()
{
	SampleRate = 22050;
	BitsPerSample = 16;
	Channels = 1;
}
