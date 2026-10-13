#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PythonTypes.h"
#include "TensorFlowTestReceiver.generated.h"

/** Collects a TensorFlow component's events for the automation tests (game thread). */
UCLASS()
class UTensorFlowTestReceiver : public UObject
{
	GENERATED_BODY()
public:
	int32 Started = 0;
	int32 Setups = 0;
	TArray<FString> Errors;
	TArray<TPair<FString, FString>> Results; //FunctionName, JsonResult
	TArray<FString> Progress;
	TArray<TPair<FString, bool>> Completions; //JsonSummary, bStoppedEarly
	TArray<TPair<FString, FString>> CustomEvents; //EventName, Data

	UFUNCTION()
	void OnStarted(const FString& Script) { ++Started; }

	UFUNCTION()
	void OnError(const FString& Message, const FString& Traceback) { Errors.Add(Message + TEXT("\n") + Traceback); }

	UFUNCTION()
	void OnSetupComplete() { ++Setups; }

	UFUNCTION()
	void OnInputResults(const FString& JsonResult, const FString& FunctionName) { Results.Emplace(FunctionName, JsonResult); }

	UFUNCTION()
	void OnTrainingProgress(const FString& Json) { Progress.Add(Json); }

	UFUNCTION()
	void OnTrainingComplete(const FString& JsonSummary, bool bStoppedEarly) { Completions.Emplace(JsonSummary, bStoppedEarly); }

	UFUNCTION()
	void OnCustomEvent(const FString& EventName, const FString& Data) { CustomEvents.Emplace(EventName, Data); }

	const TPair<FString, FString>* FindEvent(const FString& Name) const
	{
		return CustomEvents.FindByPredicate([&Name](const TPair<FString, FString>& E) { return E.Key == Name; });
	}
};
