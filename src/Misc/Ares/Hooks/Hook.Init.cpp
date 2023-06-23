#include <New/Type/TheaterTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <Misc/AresData.h>

DEFINE_OVERRIDE_HOOK(0x52BA78, _YR_GameInit_Pre, 5)
{
	TheaterTypeClass::LoadAllTheatersToArray();

	// issue #198: animate the paradrop cursor
	MouseCursor::GetCursor(MouseCursorType::ParaDrop).FrameRate = 4;

	// issue #214: also animate the chronosphere cursor
	MouseCursor::GetCursor(MouseCursorType::Chronosphere).FrameRate = 4;

	// issue #1380: the iron curtain cursor
	MouseCursor::GetCursor(MouseCursorType::IronCurtain).FrameRate = 4;

	// animate the engineer damage cursor
	MouseCursor::GetCursor(MouseCursorType::Detonate).FrameRate = 4;

	AresData::MouseCursorTypeLoadDefault();

	//Load the default regardless
	CursorTypeClass::AddDefaults();

	return 0;
}