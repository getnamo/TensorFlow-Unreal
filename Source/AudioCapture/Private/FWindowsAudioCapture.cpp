#include "AudioCapturePrivatePCH.h"
#include "LambdaRunnable.h"
#include "FWindowsAudioCapture.h"

#include "AllowWindowsPlatformTypes.h"
#include <iostream>
#include <MMSystem.h>
#include <Windows.h>
using namespace std;

#pragma comment(lib, "winmm.lib")

FWindowsAudioCapture::FWindowsAudioCapture()
{
	bRunLoopActive = false;
	SampleRate = 22050;
	//SampleRate = 44100;
}

void FWindowsAudioCapture::StartCapture(TFunction<void(const TArray<uint8>&)> OnAudioData)
{
	//Only attempt to start capture once. If it's active return.
	if (bRunLoopActive)
	{
		return;
	}

	bRunLoopActive = true;
	FThreadSafeBool* bShouldRunPtr = &bRunLoopActive;

	FLambdaRunnable::RunLambdaOnBackGroundThread([this, OnAudioData, bShouldRunPtr]()
	{
		HWAVEIN hWaveIn;
		MMRESULT result;
		WAVEFORMATEX pFormat;

		pFormat.wFormatTag = WAVE_FORMAT_PCM;
		pFormat.nChannels = 1;							//mono
		pFormat.nSamplesPerSec = SampleRate;
		pFormat.nAvgBytesPerSec = SampleRate * 2;		// = nSamplesPerSec * n.Channels * wBitsPerSample/8
		pFormat.nBlockAlign = 2;						// = n.Channels * wBitsPerSample/8
		pFormat.wBitsPerSample = 16;					// 16 for high quality, 8 for telephone-grade
		pFormat.cbSize = 0;

		result = waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);

		WAVEHDR hWaveInHdr;
		TArray<uint8> AudioBuffer;
		AudioBuffer.SetNum(SampleRate / 2);				//half a second of mono

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

		//The headers will now get filled and we should check them periodically for new data
		while (*bShouldRunPtr)
		{
			if (hWaveInHdr.dwFlags & WHDR_DONE)
			{
				TArray<uint8> OutData;
				OutData.AddUninitialized(AudioBuffer.Num());
				FMemory::Memcpy(OutData.GetData(), AudioBuffer.GetData(), AudioBuffer.Num());
				OnAudioData(OutData);
				
				//Clear flags
				hWaveInHdr.dwFlags = 0;
				hWaveInHdr.dwBytesRecorded = 0;
				
				//Re-prep
				waveInPrepareHeader(hWaveIn, &hWaveInHdr, sizeof(WAVEHDR));
				waveInAddBuffer(hWaveIn, &hWaveInHdr, sizeof(WAVEHDR));
			}
		}

		waveInStop(hWaveIn);
		waveInUnprepareHeader(hWaveIn, &hWaveInHdr, sizeof(WAVEHDR));
		waveInClose(hWaveIn);
	});
}

void FWindowsAudioCapture::StopCapture()
{
	bRunLoopActive = false;
}

#include "HideWindowsPlatformTypes.h"