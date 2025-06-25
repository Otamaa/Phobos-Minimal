#include "Multithread.h"

#include <Helpers/Macro.h>
#include <Utilities/Debug.h>
#include <Phobos.h>

#include <GScreenClass.h>
#include <WWMouseClass.h>
#include <TacticalClass.h>
#include <GadgetClass.h>
#include <MessageListClass.h>
#include <CCToolTip.h>
#include <MouseClass.h>
#include <SessionClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/Scenario/Body.h>

// Wait this long in LockOrDemandMutex before getting impatient. Bigger values = less frequent lock demands.
const std::chrono::duration MainPatienceDuration = std::chrono::milliseconds(5);
// Wait this long in LockOrDemandMutex before getting impatient. Bigger values = less frequent lock demands.
const std::chrono::duration DrawingPatienceDuration = std::chrono::milliseconds(5);
// Check this often if the demanded mutex lock has been released by the other thread.
const std::chrono::duration ChillingDuration = std::chrono::milliseconds(1);

std::unique_ptr<std::thread> Multithreading::DrawingThread = nullptr;
std::timed_mutex Multithreading::DrawingMutex;
std::mutex Multithreading::PauseMutex;
bool Multithreading::MainThreadDemandsDrawingMutex = false;
bool Multithreading::DrawingThreadDemandsDrawingMutex = false;
bool Multithreading::MainThreadDemandsPauseMutex = false;
bool Multithreading::IsInMultithreadMode = false;

namespace MessageTemp
{
	bool OnMessages = false;
	bool NewMsgList = false;
}

bool MouseIsOverMessageLists()
{
	const auto pMousePosition = &WWMouseClass::Instance->XY1;
	const auto pMessages = ScenarioExtData::Instance()->NewMessageList.get();

	if (TextLabelClass* pText = pMessages->MessageList)
	{
		if (pMousePosition->Y >= pMessages->MessagePos.Y && pMousePosition->X >= pMessages->MessagePos.X && pMousePosition->X <= pMessages->MessagePos.X + pMessages->Width)
		{
			const int textHeight = pMessages->Height;
			int height = pMessages->MessagePos.Y;

			for (; pText; pText = static_cast<TextLabelClass*>(pText->GetNext()))
				height += textHeight;

			if (pMousePosition->Y < (height + 2))
				return true;
		}
	}

	return false;
}

ASMJIT_PATCH(0x69300B, ScrollClass_MouseUpdate_SkipMouseActionUpdate, 0x6)
{
	if (Phobos::Config::MessageDisplayInCenter)
		MessageTemp::OnMessages = MouseIsOverMessageLists();

	return 0;
}

ASMJIT_PATCH(0x55DDA0, MainLoop_FrameStep_NewMessageListManage, 0x5)
{
	if (!MessageTemp::OnMessages) {
		if (const auto pList = ScenarioExtData::Instance()->NewMessageList.get())
			pList->Manage();
	}

	return 0;

}

ASMJIT_PATCH(0x6E0DD7, TActionClass_Text_Trigger, 0x5)
{

	if (const auto pList = ScenarioExtData::Instance()->NewMessageList.get()) {
		R->ECX(pList);
	}

	return 0;
}

ASMJIT_PATCH(0x623A9F, DSurface_sub_623880_DrawBitFontStrings, 0x5)
{
	if (!MessageTemp::NewMsgList)
		return 0;

	enum { SkipGameCode = 0x623AAB };

	GET(RectangleStruct* const, pRect, EAX);
	GET(DSurface* const, pSurface, ECX);
	GET(const int, height, EBP);

	pRect->Height = height;
	auto black = ColorStruct { 0, 0, 0 };
	auto trans = (MessageTemp::OnMessages || ScenarioClass::Instance->UserInputLocked) ? 80 : 40;
	pSurface->Fill_Rect_Trans(pRect, &black, trans);

	return SkipGameCode;
}

class NOVTABLE FakeGScreenClass final : GScreenClass
{
public:
	static COMPILETIMEEVAL constant_ptr<FakeGScreenClass, 0x87F7E8u> const Instance {};
	static FORCEDINLINE void _RenderRaw(GScreenClass* pThis)
	{
		auto pTempSurface = DSurface::Temp.get();
		DSurface::Temp = DSurface::Composite;
		WWMouseClass::Instance->func_40(DSurface::Composite, false);

		bool shouldDraw = pThis->Bitfield != 0;
		bool complete = pThis->Bitfield == 2;
		pThis->Bitfield = 0;

		if (!Multithreading::IonStormClass_ChronoScreenEffect_Status)
		{
			TacticalClass::Instance->Render(DSurface::Composite, shouldDraw, 0);
			TacticalClass::Instance->Render(DSurface::Composite, shouldDraw, 1);
			pThis->Draw(complete);
			TacticalClass::Instance->Render(DSurface::Composite, shouldDraw, 2);
		}

		if (Multithreading::BlitMouse.get() && !Unsorted::ArmageddonMode)
		{
			WWMouseClass::Instance->func_40(DSurface::Sidebar, true);
			Multithreading::BlitMouse = false;
		}

		if (Multithreading::Buttons.get())
			Multithreading::Buttons->DrawAll(false);

		MessageListClass::Instance->Draw();
		if (Phobos::Config::MessageDisplayInCenter) {
			MessageTemp::NewMsgList = true;
			ScenarioExtData::Instance()->NewMessageList->Draw();
			MessageTemp::NewMsgList = false;
		}

		if (CCToolTip::Instance.get())
			CCToolTip::Instance->Draw(false);

		if (Multithreading::EnableMultiplayerDebug.get())
			Multithreading::MultiplayerDebugPrint();

		Phobos::DrawVersionWarning();
		HugeBar::ProcessHugeBar();



		WWMouseClass::Instance->func_3C(DSurface::Composite, false);
		pThis->vt_entry_44();

		DSurface::Temp = pTempSurface;
	}

	void _Render()
	{
		if (Multithreading::IsInMultithreadMode)
			return;

		FakeGScreenClass::_RenderRaw(this);
	}
};
static_assert(sizeof(FakeGScreenClass) == sizeof(GScreenClass), " Invalid Size !");

void Multithreading::EnterMultithreadMode()
{
	if (IsInMultithreadMode)
		return;
	Debug::LogInfo("Entering the multithread mode - just before MainLoop gameplay loop.");
	IsInMultithreadMode = true;
	DrawingThread = std::make_unique<std::thread>(std::thread(DrawingLoop));
}

void Multithreading::ExitMultithreadMode()
{
	if (!IsInMultithreadMode)
		return;
	Debug::LogInfo("Exiting the multithread mode - MainLoop reported end of gameplay.");
	IsInMultithreadMode = false;
	DrawingThread.get()->detach();
	DrawingThread.release();
}

void Multithreading::LockOrDemandMutex(std::timed_mutex& mutex, bool& demands, std::chrono::duration<long long, std::milli> patienceDuration)
{
	if (mutex.try_lock_for(patienceDuration))
		return;
	demands = true;
	mutex.lock();
}

void Multithreading::DrawingLoop()
{
	while (Multithreading::IsInMultithreadMode)
	{
		// Main thread wants to pause the game. Let it lock the mutex by not doing anything yourself.
		while (Multithreading::MainThreadDemandsPauseMutex)
			std::this_thread::sleep_for(ChillingDuration);

		// If the game is paused, wait until it's unpaused (aka until the mutex is free to be locked).
		PauseMutex.lock();

		// We must avoid starving out the main thread...
		while (Multithreading::MainThreadDemandsDrawingMutex)
			std::this_thread::sleep_for(ChillingDuration);

		// ...but we also don't want to run 1200 TPS with 10 FPS.
		// Let's try waiting for the lock until we get impatient and *demand* priority.
		LockOrDemandMutex(Multithreading::DrawingMutex, Multithreading::DrawingThreadDemandsDrawingMutex, DrawingPatienceDuration);

		// Do the thing.
		FakeGScreenClass::_RenderRaw(GScreenClass::Instance());

		// We're done. Unlock all mutexes.
		Multithreading::DrawingThreadDemandsDrawingMutex = false;
		Multithreading::DrawingMutex.unlock();
		Multithreading::PauseMutex.unlock();
	}
	Debug::LogInfo("Exiting the drawing thread.");
}

DEFINE_DYNAMIC_PATCH(Disable_MainLoop_StartLock, 0x55D878,
	0x8B, 0x0D, 0xF8, 0xD5, 0xA8, 0x00);
DEFINE_DYNAMIC_PATCH(Disable_MainLoop_StartLock_2, 0x55DBC3,
	0xB9, 0x90, 0x03, 0x8A, 0x00);
DEFINE_DYNAMIC_PATCH(Disable_MainLoop_StopLock, 0x55DDAA,
	0xA1, 0x38, 0xB2, 0xAB, 0x00);
DEFINE_DYNAMIC_PATCH(Disable_MainLoop_StopLock_2, 0x55D903,
	0xC6, 0x05, 0x9D, 0xED, 0xA8, 0x00, 0x00);
DEFINE_DYNAMIC_PATCH(Disable_PauseGame_SetPause, 0x683EB6,
	0x8B, 0x0D, 0x58, 0xE7, 0x87, 0x00);
DEFINE_DYNAMIC_PATCH(Disable_PauseGame_ResetPause, 0x683FB2,
	0xB9, 0xE8, 0xF7, 0x87, 0x00);
DEFINE_DYNAMIC_PATCH(Disable_MainGame_BeforeMainLoop, 0x48CE7E,
	0xC6, 0x05, 0xFC, 0x2C, 0x82, 0x00, 0x01);

#ifndef _ENABLETHESE
// Disable the hooks if we're in multiplayer modes or if multithreading was disabled in rules.
ASMJIT_PATCH(0x48CE7E, MainGame_BeforeMainLoop, 7)
{
	if (Phobos::Config::MultiThreadSinglePlayer && SessionClass::Instance->IsSingleplayer())
		return 0;

	Phobos::Config::MultiThreadSinglePlayer = false;

	Disable_MainLoop_StartLock->Apply();
	Disable_MainLoop_StartLock_2->Apply();
	Disable_MainLoop_StopLock->Apply();
	Disable_MainLoop_StopLock_2->Apply();
	Disable_PauseGame_SetPause->Apply();
	Disable_PauseGame_ResetPause->Apply();
	Disable_MainGame_BeforeMainLoop->Apply();

	return 0;
}


// Completely skip vanilla GScreenClass::Render code in the main thread
// if we run in multithread mode.

DEFINE_FUNCTION_JUMP(LJMP, 0x4F4480 , FakeGScreenClass::_Render);

// We want to lock access to game resources when we're doing game logic potientially related to graphics.
// The main thread should let the drawing thread run if it complains that it's too hungry and vice versa.

ASMJIT_PATCH(0x55D878, MainLoop_StartLock, 6)
{
	if(R->Origin() == 0x55DBC3)
		DisplayClass::GetLayer(Layer::Air)->Sort();

	if (!Multithreading::IsInMultithreadMode)
		return 0;

	while (Multithreading::DrawingThreadDemandsDrawingMutex)
		std::this_thread::sleep_for(ChillingDuration);

	Multithreading::LockOrDemandMutex(
		Multithreading::DrawingMutex,
		Multithreading::MainThreadDemandsDrawingMutex,
		MainPatienceDuration);

	Multithreading::MainThreadDemandsDrawingMutex = false;

	return 0;
}ASMJIT_PATCH_AGAIN(0x55DBC3, MainLoop_StartLock, 5)

// See above.
ASMJIT_PATCH(0x55DDAA, MainLoop_StopLock, 5)
{
	if (!Multithreading::IsInMultithreadMode)
		return 0;

	Multithreading::DrawingMutex.unlock(); // TODO: shut up the warning
	return 0;
}ASMJIT_PATCH_AGAIN(0x55D903, MainLoop_StopLock, 7)


// We don't want to draw the tactical view when the player has paused the game.
// Technically it can be done with a busy wait loop, but mutexes are better for performance.
// The main thread should always have priority for the mutex access.
ASMJIT_PATCH(0x683EB6, PauseGame_SetPause, 6)
{
	if (!Multithreading::IsInMultithreadMode)
		return 0;

	Multithreading::MainThreadDemandsPauseMutex = true;
	Multithreading::PauseMutex.lock();
	Multithreading::MainThreadDemandsPauseMutex = false;
	return 0;
}

// Resume operation after pausing the game.
ASMJIT_PATCH(0x683FB2, ResumeGame_ResetPause, 5)
{
	if (!Multithreading::IsInMultithreadMode)
		return 0;

	Multithreading::PauseMutex.unlock();
	return 0;
}
#endif