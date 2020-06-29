#include "FWindowsAudioCapture.h"
#include "CULambdaRunnable.h"
#include "CoreMinimal.h"
#include "HAL/ThreadSafeBool.h"

#include "AllowWindowsPlatformTypes.h"
#include <iostream>
#include <MMSystem.h>
#include <Windows.h>
using namespace std;

#pragma comment(lib, "winmm.lib")

FWindowsAudioCapture::FWindowsAudioCapture()
{
	bRunLoopActive = false;
}

void FWindowsAudioCapture::StartCapture(TFunction<void(const TArray<uint8>&, float)> OnAudioData /*= nullptr*/, TFunction<void(const TArray<uint8>&, float)> OnCaptureFinished /*= nullptr*/)
{
	//Only attempt to start capture once. If it's active return.
	if (bRunLoopActive)
	{
		return;
	}

	bRunLoopActive = true;
	FThreadSafeBool* bShouldRunPtr = &bRunLoopActive;

	FCULambdaRunnable::RunLambdaOnBackGroundThread([this, OnAudioData, OnCaptureFinished, bShouldRunPtr]()
	{
		HWAVEIN hWaveIn;
		MMRESULT result;
		WAVEFORMATEX pFormat;

		pFormat.wFormatTag = WAVE_FORMAT_PCM;
		pFormat.nChannels = Options.Channels;									//typically 1 or 2
		pFormat.nSamplesPerSec = Options.SampleRate;
		pFormat.wBitsPerSample = Options.BitsPerSample;							// 16 for high quality, 8 for telephone-grade
		pFormat.nBlockAlign = pFormat.nChannels * pFormat.wBitsPerSample / 8;	// = n.Channels * wBitsPerSample/8
		pFormat.nAvgBytesPerSec = Options.SampleRate * pFormat.nBlockAlign;		// = nSamplesPerSec * n.Channels * wBitsPerSample/8

		pFormat.cbSize = 0;

		result = waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);

		WAVEHDR hWaveInHdr;
		TArray<uint8> AudioBuffer;
		AudioBuffer.SetNum(pFormat.nAvgBytesPerSec / 2);		//half a second

		hWaveInHdr.lpData = (LPSTR)AudioBuffer.GetData();
		hWaveInHdr.dwBufferLength = AudioBuffer.Num();
		hWaveInHdr.dwBytesRecorded = 0;
		hWaveInHdr.dwUser = 0L;
		hWaveInHdr.dwFlags = 0L;
		hWaveInHdr.dwLoops = 0L;

		waveInPrepareHeader(hWaveIn, &hWaveInHdr, sizeof(WAVEHDR));

		// Insert a wave input buffer
		result = waveInAddBuffer(hWaveIn, &hWaveInHdr, sizeof(WAVEHDR));
		result = waveInStart(hWaveIn);

		float MaxLevel = 0.f;

		//The headers will now get filled and we should check them periodically for new data
		while (*bShouldRunPtr)
		{
			if (hWaveInHdr.dwFlags & WHDR_DONE)
			{
				TArray<uint8> OutData;
				OutData.SetNum(hWaveInHdr.dwBytesRecorded);
				FMemory::Memcpy(OutData.GetData(), AudioBuffer.GetData(), hWaveInHdr.dwBytesRecorded);

				MaxLevel = CalculateMaxAudioLevel(OutData, pFormat.wBitsPerSample);

				if (OnAudioData != nullptr)
				{
					OnAudioData(OutData, MaxLevel);
				}

				//Clear flags
				hWaveInHdr.dwFlags = 0;
				hWaveInHdr.dwBytesRecorded = 0;
				
				//Re-prep
				waveInPrepareHeader(hWaveIn, &hWaveInHdr, sizeof(WAVEHDR));
				waveInAddBuffer(hWaveIn, &hWaveInHdr, sizeof(WAVEHDR));
			}
		}

		//Stop
		waveInStop(hWaveIn);

		//Clear flags
		hWaveInHdr.dwFlags = 0;
		hWaveInHdr.dwBytesRecorded = 0;

		//Flush last data
		waveInPrepareHeader(hWaveIn, &hWaveInHdr, sizeof(WAVEHDR));
		waveInAddBuffer(hWaveIn, &hWaveInHdr, sizeof(WAVEHDR));

		TArray<uint8> OutData;
		OutData.SetNum(hWaveInHdr.dwBytesRecorded);
		FMemory::Memcpy(OutData.GetData(), AudioBuffer.GetData(), hWaveInHdr.dwBytesRecorded);
		MaxLevel = CalculateMaxAudioLevel(OutData, pFormat.wBitsPerSample);

		waveInUnprepareHeader(hWaveIn, &hWaveInHdr, sizeof(WAVEHDR));
		waveInClose(hWaveIn);

		if (OnAudioData != nullptr)
		{
			OnAudioData(OutData, MaxLevel);
		}

		//flush whatever is left of the buffer, emit both in data and on finished
		if (OnCaptureFinished != nullptr)
		{
			OnCaptureFinished(OutData, MaxLevel);
		}
	});
}

void FWindowsAudioCapture::StopCapture()
{
	bRunLoopActive = false;
}

void FWindowsAudioCapture::SetOptions(const FAudioCaptureOptions& InOptions)
{
	Options = InOptions;
}

float FWindowsAudioCapture::CalculateMaxAudioLevel(TArray<uint8>& Buffer, int32 BitsPerSample)
{
	//Can't handle non-16 sample
	if(BitsPerSample != 16)
	{
		return 0.5f;
	}

	int32 Num = (Buffer.Num() / BitsPerSample) * 8;
	int16* Buffer16 = (int16*)Buffer.GetData();
	int16 MaxValue = 0;
	for (int i = 0; i < Num; i++)
	{
		int16 Value = FMath::Abs(Buffer16[i]);
		if (Value > MaxValue)
		{
			MaxValue = Value;
		}
	}
	//UE_LOG(LogTemp, Log, TEXT("max value: %d"), MaxValue);

	return (float)MaxValue / (float)INT16_MAX;
}

#include "HideWindowsPlatformTypes.h"