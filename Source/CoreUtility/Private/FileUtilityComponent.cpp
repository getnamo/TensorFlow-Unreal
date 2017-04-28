#include "CoreUtilityPrivatePCH.h"
#include "FileUtilityComponent.h"

UFileUtilityComponent::UFileUtilityComponent(const FObjectInitializer &init) : UActorComponent(init)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;
}

FString UFileUtilityComponent::ProjectDirectory()
{
	return FPaths::GameContentDir();
}

bool UFileUtilityComponent::SaveBytesToFile(const TArray<uint8>& Bytes, const FString& Directory, const FString& FileName)
{
	//bool AllowOverwriting = false;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// CreateDirectoryTree returns true if the destination
	// directory existed prior to call or has been created
	// during the call.
	if (PlatformFile.CreateDirectoryTree(*Directory))
	{
		// Get absolute file path
		FString AbsoluteFilePath = Directory + "/" + FileName;

		// Allow overwriting or file doesn't already exist
		FFileHelper::SaveArrayToFile(Bytes, *AbsoluteFilePath);
	}

	return false;
}
