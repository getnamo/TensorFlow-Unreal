#pragma once

#include "Components/ActorComponent.h"
#include "FileUtilityComponent.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudioDataSignature, const TArray<uint8>&, Bytes);

/**
*
*/
UCLASS(ClassGroup = "Utility", meta = (BlueprintSpawnableComponent))
class COREUTILITY_API UFileUtilityComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
public:

	/** Get the current project contents directory path*/
	UFUNCTION(BlueprintPure, Category = FileUtility)
	FString ProjectContentsDirectory();

	/** Save array of bytes to file at specified directory */
	UFUNCTION(BlueprintCallable, Category = FileUtility)
	bool SaveBytesToFile(const TArray<uint8>& Bytes, const FString& Directory, const FString& FileName);

	/** Read array of bytes from file at specified directory */
	UFUNCTION(BlueprintCallable, Category = FileUtility)
	bool ReadBytesFromFile(const FString& Directory, const FString& FileName, TArray<uint8>& OutBytes);
};