#include "CoreUtilityPrivatePCH.h"
#include "LambdaRunnable.h"

uint64 FLambdaRunnable::ThreadNumber = 0;

FLambdaRunnable::FLambdaRunnable(TFunction< void()> InFunction)
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

FLambdaRunnable::~FLambdaRunnable()
{
	if (Thread == NULL)
	{
		delete Thread;
		Thread = NULL;
	}

	//Runnables.Remove(this);
}

//Init
bool FLambdaRunnable::Init()
{
	//UE_LOG(CoreUtilityLog, Log, TEXT("FLambdaRunnable %d Init"), Number);
	return true;
}

//Run
uint32 FLambdaRunnable::Run()
{
	if (FunctionPointer != nullptr)
	{
		FunctionPointer();
	}
	
	//UE_LOG(CoreUtilityLog, Log, TEXT("FLambdaRunnable %d Run complete"), Number);
	return 0;
}

//stop
void FLambdaRunnable::Stop()
{
	Finished = true;
}

void FLambdaRunnable::Kill()
{
	Thread->Kill(false);
	Finished = true;
}

void FLambdaRunnable::Exit()
{
	Finished = true;
	//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable %d Exit"), Number);

	//delete ourselves when we're done
	delete this;
}

void FLambdaRunnable::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

FLambdaRunnable* FLambdaRunnable::RunLambdaOnBackGroundThread(TFunction< void()> InFunction)
{
	FLambdaRunnable* Runnable;
	if (FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FLambdaRunnable(InFunction);
		//UE_LOG(LogClass, Log, TEXT("FLambdaRunnable RunLambdaBackGroundThread"));
		return Runnable;
	}
	else
	{
		return nullptr;
	}
}

FGraphEventRef FLambdaRunnable::RunShortLambdaOnGameThread(TFunction< void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}

void FLambdaRunnable::ShutdownThreads()
{
	/*for (auto Runnable : Runnables)
	{
	if (Runnable != nullptr)
	{
		Runnable.Stop();
		delete Runnable;
	}
	Runnable = nullptr;*/
}