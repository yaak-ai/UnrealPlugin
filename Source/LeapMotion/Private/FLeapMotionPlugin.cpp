// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#include "FLeapMotionPlugin.h"
#include "FLeapMotionInputDevice.h"
#include "BodyStateBPLibrary.h"
#include "IInputDeviceModule.h"
#include "Interfaces/IPluginManager.h"
#include "BodyStateDeviceConfig.h"
#include "Modules/ModuleManager.h"


#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include <objbase.h>
#include <assert.h>
#include <stdio.h>
#include "Windows/PostWindowsApi.h"
#include "Windows/MinWindows.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#define LOCTEXT_NAMESPACE "LeapPlugin"

void FLeapMotionPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	// Custom module-specific init can go here.

	// IMPORTANT: This line registers our input device module with the engine.
	//	      If we do not register the input device module with the engine,
	//	      the engine won't know about our existence. Which means 
	//	      CreateInputDevice never gets called, which means the engine
	//	      will never try to poll for events from our custom input device.

	//Load the dll from appropriate location
	LeapDLLHandle = GetLeapHandle();

	IModularFeatures::Get().RegisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);

	//Get and display our plugin version in the log for debugging
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FString("LeapMotion"));
	const FPluginDescriptor& PluginDescriptor = Plugin->GetDescriptor();
	UE_LOG(LeapMotionLog, Log, TEXT("Leap Plugin started v%s"), *PluginDescriptor.VersionName);
}

void FLeapMotionPlugin::ShutdownModule()
{
	UE_LOG(LeapMotionLog, Log, TEXT("Leap Plugin shutdown."));

	if (LeapDLLHandle)
	{
		FPlatformProcess::FreeDllHandle(LeapDLLHandle);
		LeapDLLHandle = nullptr;
	}

	// Unregister our input device module
	IModularFeatures::Get().UnregisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
}


void FLeapMotionPlugin::AddEventDelegate(const ULeapComponent* EventDelegate)
{
	if (bActive)
	{
		LeapInputDevice->AddEventDelegate(EventDelegate);
	}
	else 
	{
		//Delay load it until it is ready
		DeferredComponentList.Add((ULeapComponent*)EventDelegate);
	}
}


void FLeapMotionPlugin::RemoveEventDelegate(const ULeapComponent* EventDelegate)
{
	if (bActive)
	{
		LeapInputDevice->RemoveEventDelegate(EventDelegate);
	}
}

FLeapStats FLeapMotionPlugin::GetLeapStats()
{
	if (bActive)
	{
		return LeapInputDevice->GetStats();
	}
	else
	{
		return ILeapMotionPlugin::GetLeapStats();
	}
}

void FLeapMotionPlugin::SetOptions(const FLeapOptions& Options)
{
	if (bActive)
	{
		LeapInputDevice->SetOptions(Options);
	}
}

FLeapOptions FLeapMotionPlugin::GetOptions()
{
	if (bActive)
	{
		return LeapInputDevice->GetOptions();
	}
	else
	{
		return ILeapMotionPlugin::GetOptions();
	}
}

void FLeapMotionPlugin::AreHandsVisible(bool& LeftHandIsVisible, bool& RightHandIsVisible)
{
	if (bActive) 
	{
		LeapInputDevice->AreHandsVisible(LeftHandIsVisible, RightHandIsVisible);
	}
}

void FLeapMotionPlugin::GetLatestFrameData(FLeapFrameData& OutData)
{
	//Copies data
	if (bActive)
	{
		LeapInputDevice->LatestFrame(OutData);
	}
}

void FLeapMotionPlugin::SetLeapPolicy(ELeapPolicyFlag Flag, bool Enable)
{
	if (bActive)
	{
		LeapInputDevice->SetLeapPolicy(Flag, Enable);
	}
}
void FLeapMotionPlugin::GetAttachedDevices(TArray<FString>& Devices)
{
	if (bActive)
	{
		Devices = LeapInputDevice->GetAttachedDevices();
	}

}
void FLeapMotionPlugin::ShutdownLeap()
{
	if (bActive)
	{
		UE_LOG(LeapMotionLog, Log, TEXT("Shutting down leap from command."));
		LeapInputDevice->ShutdownLeap();
	}
}

void* FLeapMotionPlugin::GetLeapHandle()
{
	void* NewLeapDLLHandle = nullptr;

#if PLATFORM_WINDOWS
	const bool IsGemini = IsLeapServiceVersionGemini();
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(FString("LeapMotion"));
	// Load LeapC DLL
	FString LeapCLibraryPath;

	if (Plugin != nullptr)
	{
		FString BaseDir = Plugin->GetBaseDir();
		LeapCLibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/Win64/LeapC.dll"));

		NewLeapDLLHandle = !LeapCLibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LeapCLibraryPath) : nullptr;

	}
#endif //PLATFORM_WINDOWS

	if (NewLeapDLLHandle != nullptr)
	{
		UE_LOG(LeapMotionLog, Log, TEXT("Engine plugin DLL found at %s"), *FPaths::ConvertRelativePathToFull(LeapCLibraryPath));
	}
	return NewLeapDLLHandle;
}

TSharedPtr< class IInputDevice > FLeapMotionPlugin::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	FLeapMotionPlugin::LeapInputDevice = MakeShareable(new FLeapMotionInputDevice(InMessageHandler));

	bActive = true;

	//Add all the deferred components and empty it
	for (int i = 0; i < DeferredComponentList.Num(); i++)
	{
		AddEventDelegate(DeferredComponentList[i]);
	}
	DeferredComponentList.Empty();

	return TSharedPtr< class IInputDevice >(LeapInputDevice);
}
#if PLATFORM_WINDOWS

bool GetExecutablePathForService(const FString& ServiceName, FString& PathRet)
{
	FString RegistryPath = FString("SYSTEM\\CurrentControlSet\\Services\\") + ServiceName;
	FString ValueName("ImagePath");

	HKEY hKey;
	LONG Result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, *RegistryPath, 0, KEY_READ, &hKey);
	if (Result != 0)
	{
		return false;
	}
	TCHAR Buffer[MAX_PATH];
	DWORD BufferSize = sizeof(Buffer);
	HRESULT hResult = RegQueryValueEx(hKey, *ValueName, 0, nullptr, reinterpret_cast<LPBYTE>(Buffer), &BufferSize);
	if (hResult != 0)
	{
		return false;
	}
	PathRet = FPaths::GetPath(Buffer) + "\\" + FPaths::GetBaseFilename(Buffer) + ".exe";
	
	return true;
}
#endif //PLATFORM_WINDOWS

#define LEAP_ORION_VERSION 4
bool FLeapMotionPlugin::IsLeapServiceVersionGemini()
{
	FString Ret;
#if PLATFORM_WINDOWS
	FString Path;
	if (GetExecutablePathForService(FString("LeapService"), Path))
	{
		uint64 FileVersion = FPlatformMisc::GetFileVersion(Path);

		// each word is one point of the version
		uint32 VersionMajor = FileVersion >> 32;
		uint32 VersionMinor = FileVersion & (uint64)0xFFFFFFFF;

		uint16 VersionMajorMSW = VersionMajor >> 16;
		uint16 VersionMajorLSW = VersionMajor & (uint32)0xFFFF;

		uint16 VersionMinorMSW = VersionMinor >> 16;
		uint16 VersionMinorLSW = VersionMinor & (uint32)0xFFFF;


		UE_LOG(LeapMotionLog, Log, TEXT("Leap Service Found Version %d.%d.%d.%d"), VersionMajorMSW, VersionMajorLSW, VersionMinorMSW, VersionMinorLSW);
		// Greater than version 4?, somehow windows versions are hex encoded?
		if (VersionMajorMSW > LEAP_ORION_VERSION)
		{
			UE_LOG(LeapMotionLog, Log, TEXT("Leap Service is greater than v4"));
			return true;
		}
		else
		{
			return false;
		}

	}
#endif //PLATFORM_WINDOWS

	return true;
}
IMPLEMENT_MODULE(FLeapMotionPlugin, LeapMotion)

#undef LOCTEXT_NAMESPACE