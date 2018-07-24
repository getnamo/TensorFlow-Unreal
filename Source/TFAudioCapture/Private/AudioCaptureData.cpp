#include "AudioCaptureData.h"

FAudioCaptureOptions::FAudioCaptureOptions()
{
	//this is deep-speech default
	SampleRate = 16000;
	BitsPerSample = 16;
	Channels = 1;
}

FWavHeader::FWavHeader()
{
	//Fixed things
	memcpy((char*)RIFF, "RIFF", 4);
	memcpy((char*)WAVE, "WAVE", 4);
	memcpy((char*)fmt, "fmt ", 4);
	memcpy((char*)Subchunk2ID, "data", 4);
	Subchunk1Size = 16;

	//Default to 16khz 16 bit PCM Mono
	SetHeaderSampleData(16000, 16);
	AudioFormat = 1;	//PCM
}

void FWavHeader::SetHeaderFromOptions(const FAudioCaptureOptions& Options)
{
	SetHeaderSampleData(Options.SampleRate, Options.BitsPerSample, Options.Channels);
}

void FWavHeader::SetHeaderSampleData(int InSampleRate, int InBitsPerSample, int InChannelCount /*= 1*/)
{
	//Set direct settings
	SampleRate = InSampleRate;
	BitsPerSample = InBitsPerSample;
	ChannelCount = InChannelCount;

	int32 BytesPerSample = (int(BitsPerSample / 8));

	//Set derived settings
	BytesPerSec = (int)SampleRate * ChannelCount * BytesPerSample;
	BlockAlign = (int)BytesPerSample * ChannelCount;
}

FAudioCaptureOptions FWavHeader::ToAudioOptions()
{
	FAudioCaptureOptions Options;
	Options.SampleRate = SampleRate;
	Options.BitsPerSample = BitsPerSample;
	Options.Channels = ChannelCount;
	return Options;
}

void FWavHeader::SetBufferSize(int ByteCount)
{
	ChunkSize = 36 + ByteCount;
	Subchunk2Size = ByteCount;
}

TArray<uint8> FWavHeader::RawPCMToWav(const TArray<uint8>& RawPCMBytes, const FAudioCaptureOptions& AudioOptions)
{
	TArray<uint8> OutWavBytes;
	FWavHeader Header;

	Header.SetHeaderFromOptions(AudioOptions);
	Header.SetBufferSize(RawPCMBytes.Num());

	OutWavBytes.Append((uint8*)&Header, sizeof(Header));
	OutWavBytes.Append(RawPCMBytes);

	return OutWavBytes;
}

void FWavHeader::WavToRawPCM(const TArray<uint8>& WavBytes, TArray<uint8>& OutRawPCMBytes, FAudioCaptureOptions& OutAudioOptions)
{
	FWavHeader Header;
	int HeaderSize = sizeof(FWavHeader);

	memcpy(&Header, WavBytes.GetData(), HeaderSize);

	OutAudioOptions = Header.ToAudioOptions();

	OutRawPCMBytes.Empty();
	OutRawPCMBytes.Append(&WavBytes[HeaderSize], WavBytes.Num() - HeaderSize);
}
