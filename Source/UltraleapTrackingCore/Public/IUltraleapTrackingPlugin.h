

#pragma once

#include "IInputDeviceModule.h"
#include "UltraleapTrackingData.h"

class ULeapComponent;

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules
 * within this plugin.
 */
class ULTRALEAPTRACKING_API IUltraleapTrackingPlugin : public IInputDeviceModule
{
public:
	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IUltraleapTrackingPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked<IUltraleapTrackingPlugin>("UltraleapTracking");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("UltraleapTracking");
	}

	// These are typically called by a wrapped class such as LeapController (Actor Component)

	/** Attach an event delegate to the leap input device loop*/
	virtual void AddEventDelegate(const ULeapComponent* EventDelegate){};

	/** Remove an event delegate from the leap input device loop*/
	virtual void RemoveEventDelegate(const ULeapComponent* EventDelegate){};

	virtual FLeapStats GetLeapStats()
	{
		return FLeapStats();
	};

	/** Set Leap Options such as time warp, interpolation and tracking modes */
	virtual void SetOptions(const FLeapOptions& InOptions){};

	/** Get the currently set Leap Options */
	virtual FLeapOptions GetOptions()
	{
		return FLeapOptions();
	};

	/** Convenience function to determine hand visibility*/
	virtual void AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible) = 0;

	/** Polling method for latest frame data*/
	virtual void GetLatestFrameData(FLeapFrameData& OutData) = 0;

	/** Set a Leap Policy, such as image streaming or optimization type*/
	virtual void SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable) = 0;

	/** List the attached (plugged in) devices */
	virtual void GetAttachedDevices(TArray<FString>& Devices) = 0;

	/** Force shutdown leap, do not call unless you have a very specfic need*/
	virtual void ShutdownLeap() = 0;

	virtual void SetSwizzles(
		ELeapQuatSwizzleAxisB ToX, ELeapQuatSwizzleAxisB ToY, ELeapQuatSwizzleAxisB ToZ, ELeapQuatSwizzleAxisB ToW) = 0;
};
