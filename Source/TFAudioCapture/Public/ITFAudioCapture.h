#pragma once

#include "IAudioCaptureInterface.h"
#include "IAudioDataInterface.h"
#include "ModuleManager.h"

class UTFAudioCaptureComponent;

class TFAUDIOCAPTURE_API ITFAudioCapture : public IModuleInterface, public IAudioCaptureInterface
{
public:

	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline ITFAudioCapture& Get()
	{
		return FModuleManager::LoadModuleChecked< ITFAudioCapture >("TFAudioCapture");
	}

	/**
	* Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	*
	* @return True if the module is loaded and ready to use
	*/
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("TFAudioCapture");
	}

	//For components
	virtual void AddAudioComponent(const UTFAudioCaptureComponent* Component) {};
	virtual void RemoveAudioComponent(const UTFAudioCaptureComponent* Component) {};

	//for native delegates
	virtual void AddAudioDelegate(IAudioDataInterface* Delegate) {};
	virtual void RemoveAudioDelegate(IAudioDataInterface* Delegate) {};
};