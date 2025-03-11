#pragma once

#include <mutex>
#include <chrono>
#include <thread>

#include <Utilities/Macro.h>
#include <Helpers/CompileTime.h>

class GadgetClass;
struct Multithreading
{
	static COMPILETIMEEVAL reference<bool, 0xB0B519u> const BlitMouse {};
	static COMPILETIMEEVAL reference<bool, 0xA9FAB0u> const IonStormClass_ChronoScreenEffect_Status {};
	static COMPILETIMEEVAL reference<GadgetClass*, 0xA8EF54u> const Buttons {};
	static COMPILETIMEEVAL reference<bool, 0xA8B8B4u> const EnableMultiplayerDebug {};

	static void MultiplayerDebugPrint()
		{ JMP_STD(0x55F1E0); }

	static std::unique_ptr<std::thread> DrawingThread;
	static std::timed_mutex DrawingMutex;
	static std::mutex PauseMutex;
	static bool MainThreadDemandsDrawingMutex;
	static bool DrawingThreadDemandsDrawingMutex;
	static bool MainThreadDemandsPauseMutex;
	static bool IsInMultithreadMode;

	// Our new drawing thread loop.
	static void DrawingLoop();
	// Spawn the drawing thread.
	static void EnterMultithreadMode();
	// Kill the drawing thread.
	static void ExitMultithreadMode();
	// Wait for mutex lock respectfuly or, if too much time has passed, demand it.
	static void LockOrDemandMutex(std::timed_mutex& mutex, bool& demands, std::chrono::duration<long long, std::milli> patienceDuration);
	
	static void ShutdownMultitheadMode()
	{
		if (!IsInMultithreadMode)
			return;

		IsInMultithreadMode = false;
		DrawingThread.get()->detach();
		DrawingThread.release();
	}
};

