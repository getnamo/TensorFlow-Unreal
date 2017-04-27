#pragma once

#include "LambdaRunnablePrivatePCH.h"
#include "LambdaRunnable.h"

uint64 FLambdaRunnable::ThreadNumber = 0;

LambdaRunnable::LambdaRunnable(TFunction< void()> InFunction)
{
	FunctionPointer = InFunction;
	Finished = false;
	Number = ThreadNumber;

	FString threadStatGroup = FString::Printf(TEXT("FSIOLambdaRunnable%d"), ThreadNumber);
	Thread = NULL;
	Thread = FRunnableThread::Create(this, *threadStatGroup, 0, TPri_BelowNormal); //windows default = 8mb for thread, could specify more
	ThreadNumber++;

	//Runnables.Add(this);
}

LambdaRunnable::~LambdaRunnable()
{
	if (Thread == NULL)
	{
		delete Thread;
		Thread = NULL;
	}

	//Runnables.Remove(this);
}

//Init
bool LambdaRunnable::Init()
{
	//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable %d Init"), Number);
	return true;
}

//Run
uint32 LambdaRunnable::Run()
{
	if (FunctionPointer != nullptr)
	{
		FunctionPointer();
	}
	UE_LOG(LogClass, Log, TEXT("FLambdaRunnable %d Run complete"), Number);
	return 0;
}

//stop
void LambdaRunnable::Stop()
{
	Finished = true;
}

void LambdaRunnable::Kill()
{
	UE_LOG(LogClass, Log, TEXT("Yolo!"));
	Thread->Kill(false);
	Finished = true;
}

void LambdaRunnable::Exit()
{
	Finished = true;
	//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable %d Exit"), Number);

	//delete ourselves when we're done
	delete this;
}

void LambdaRunnable::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

LambdaRunnable* LambdaRunnable::RunLambdaOnBackGroundThread(TFunction< void()> InFunction)
{
	LambdaRunnable* Runnable;
	if (FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FSIOLambdaRunnable(InFunction);
		//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable RunLambdaBackGroundThread"));
		return Runnable;
	}
	else
	{
		return nullptr;
	}
}

FGraphEventRef LambdaRunnable::RunShortLambdaOnGameThread(TFunction< void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}

void LambdaRunnable::ShutdownThreads()
{
	/*for (auto Runnable : Runnables)
	{
	if (Runnable != nullptr)
	{
	delete Runnable;
	}
	Runnable = nullptr;
	}*/
}