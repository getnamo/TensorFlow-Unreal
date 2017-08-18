#pragma once

#include "AudioCaptureData.generated.h"

USTRUCT(BlueprintType)
struct TFAUDIOCAPTURE_API FAudioCaptureOptions
{
	GENERATED_USTRUCT_BODY()
	FAudioCaptureOptions();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Capture Options")
	int32 SampleRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Capture Options")
	int32 BitsPerSample;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Capture Options")
	int32 Channels;
};

/** Wav format Header for wav files embedding raw PCM*/
typedef struct FWavHeader
{
	/* RIFF Chunk Descriptor */
	uint8_t			RIFF[4];        // RIFF Header Magic header
	uint32_t        ChunkSize;      // RIFF Chunk Size
	uint8_t			WAVE[4];        // WAVE Header
									/* "fmt" sub-chunk */
	uint8_t			fmt[4];         // FMT header
	uint32_t        Subchunk1Size;  // Size of the fmt chunk
	uint16_t        AudioFormat;    // Audio format 1=PCM, 6=mulaw, 7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	uint16_t        ChannelCount;   // Number of channels 1=Mono 2=Stereo
	uint32_t        SampleRate;		// Sampling Frequency in Hz
	uint32_t        BytesPerSec;    // bytes per second
	uint16_t        BlockAlign;     // 2=16-bit mono, 4=16-bit stereo
	uint16_t        BitsPerSample;  // Number of bits per sample
									/* "data" sub-chunk */
	uint8_t			Subchunk2ID[4]; // "data"  string
	uint32_t        Subchunk2Size;  // Sampled data length

	/** default construct presets a couple of things by default */
	FWavHeader();

	/** Convenience wrapper to fill the wav header sample data*/
	void SetHeaderFromOptions(const FAudioCaptureOptions& Options);
	void SetHeaderSampleData(int InSampleRate, int InBitsPerSample, int InChannelCount = 1);
	
	FAudioCaptureOptions ToAudioOptions();

	/** Will correctly fill*/
	void SetBufferSize(int ByteCount);

	/** Pass raw PCM bytes and audio options to receive encoded wav bytes */
	static TArray<uint8> RawPCMToWav(const TArray<uint8>& InRawPCMBytes, const FAudioCaptureOptions& AudioOptions);
	static void WavToRawPCM(const TArray<uint8>& WavBytes, TArray<uint8>& OutRawPCMBytes, FAudioCaptureOptions& OutAudioOptions);

};