#pragma once
#include "CoreUtilityPrivatePCH.h"

class FCoreUtility : public ICoreUtility
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() {};
	virtual void ShutdownModule() {};
};


IMPLEMENT_MODULE(FCoreUtility, CoreUtility)