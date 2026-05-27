#include "FrameStep.h"

#include <SessionClass.h>
#include <TacticalClass.h>

size_t FrameByFrameCommandClass::FrameStepCount = 0;
bool FrameByFrameCommandClass::FrameStep = false;

const char* FrameByFrameCommandClass::GetName() const
{
	return "FrameByFrame";
}

const wchar_t* FrameByFrameCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_FRAME_BY_FRAME", L"Toggle Frame By Frame");
}

const wchar_t* FrameByFrameCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* FrameByFrameCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DISPLAY_DAMAGE_DESC", L"Enter or exit frame by frame mode.");
}

void FrameByFrameCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	if (!SessionClass::Instance->IsSingleplayer())
		return;

	if (!FrameStep)
		Debug::LogAndMessage("Entering Stepping Mode...");
	else
		Debug::LogAndMessage("Exiting Stepping Mode...");

	FrameStep = !FrameStep;
	FrameStepCount = 0;
}

bool FrameByFrameCommandClass::FrameStepMainLoop()
{
	if (Game::IsActive.get())
	{
		Game::CallBack();

		if (Game::IsFocused.get() && Game::SpecialDialog.get() == 0)
		{
			MapClass::Instance->MarkNeedsRedraw(2);

			DWORD input;
			int x, y;
			MapClass::Instance->GetInputAndUpdate(input, x, y);
			if (input != NULL)
			{
				Game::KeyboardProcess(input);

				// Allow to open options
				if (input == VK_ESCAPE || input == VK_SPACE)
					Game::SpecialDialog = 1;
			}

			MapClass::Instance->Render();
			TacticalClass::Instance->Update();
		}
	}

	Sleep(1);
	return Game::IsActive.get() == false;
}

ASMJIT_PATCH(0x55D360, MainLoop_FrameStep_Begin, 0x5)
{
	// FrameStep mode enabled but no frames to process
	if (FrameByFrameCommandClass::FrameStep && FrameByFrameCommandClass::FrameStepCount == 0)
	{
		R->EAX(FrameByFrameCommandClass::FrameStepMainLoop());
		return 0x55DEDB;
	}

	// This frame need to be processed
	return 0;
}

ASMJIT_PATCH(0x55DED5, MainLoop_FrameStep_End, 0x6)
{
	// This frame is processed, decrease the counter
	if (FrameByFrameCommandClass::FrameStep && FrameByFrameCommandClass::FrameStepCount > 0)
		--FrameByFrameCommandClass::FrameStepCount;

	return 0;
}ASMJIT_PATCH_AGAIN(0x55D871, MainLoop_FrameStep_End, 0x6)
ASMJIT_PATCH_AGAIN(0x55DEC1, MainLoop_FrameStep_End, 0x6)


template<size_t Frame>
class FrameStepCommandClass : public PhobosCommandClass
{
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

template<size_t Frame>
OPTIONALINLINE const char* FrameStepCommandClass<Frame>::GetName() const
{
	IMPL_SNPRNINTF(Phobos::readBuffer, Phobos::readLength, "Step%dFrames", Frame);
	return Phobos::readBuffer;
}

template<size_t Frame>
OPTIONALINLINE const wchar_t* FrameStepCommandClass<Frame>::GetUIName() const
{
	const wchar_t* csfString = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_STEP_XX_FORWARD", L"Step Forward %d Frames");
	_snwprintf_s(Phobos::wideBuffer, std::size(Phobos::wideBuffer), csfString, Frame);
	return Phobos::wideBuffer;
}

template<size_t Frame>
OPTIONALINLINE const wchar_t* FrameStepCommandClass<Frame>::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

template<size_t Frame>
OPTIONALINLINE const wchar_t* FrameStepCommandClass<Frame>::GetUIDescription() const
{
	const wchar_t* csfString = GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_STEP_XX_FORWARD_DESC", L"Frame Step Only: Step forward %d frames.");
	_snwprintf_s(Phobos::wideBuffer, std::size(Phobos::wideBuffer), csfString, Frame);
	return Phobos::wideBuffer;
}

template<size_t Frame>
OPTIONALINLINE void FrameStepCommandClass<Frame>::Execute(WWKey eInput) const
{
	if (!FrameByFrameCommandClass::FrameStep)
		return;

	Debug::LogAndMessage("Stepping {} frames...", Frame);

	FrameByFrameCommandClass::FrameStepCount = Frame;
}

template <typename T>
static FORCEDINLINE T* Make()
{
	T* command = GameCreate<T>();
	CommandClass::Array->push_back(command);
	return command;
};

void FrameStepDispatch::Dispatch()
{
	Make<FrameStepCommandClass<1>>(); // Single step in
	Make<FrameStepCommandClass<5>>(); // Speed 1
	Make<FrameStepCommandClass<10>>(); // Speed 2
	Make<FrameStepCommandClass<15>>(); // Speed 3
	Make<FrameStepCommandClass<30>>(); // Speed 4
	Make<FrameStepCommandClass<60>>(); // Speed 5
}
