#pragma once

#include "Commands.h"
#include <Utilities/Debug.h>
#include <Utilities/GeneralUtils.h>

class FrameByFrameCommandClass : public PhobosCommandClass
{
public:
	static size_t FrameStepCount;
	static bool FrameStep;

	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;

	static bool FrameStepMainLoop();
};

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