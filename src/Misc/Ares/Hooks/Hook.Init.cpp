#include <New/Type/TheaterTypeClass.h>
#include <New/Type/CursorTypeClass.h>

#include <ParticleClass.h>

DEFINE_OVERRIDE_HOOK(0x52BA78, _YR_GameInit_Pre, 5)
{
	TheaterTypeClass::LoadAllTheatersToArray();

	MouseCursor::GetCursor(MouseCursorType::ParaDrop).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Chronosphere).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::IronCurtain).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Detonate).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::Cursor_36).FrameRate = 4;
	MouseCursor::GetCursor(MouseCursorType::IvanBomb).FrameRate = 4;

	//AresData::MouseCursorTypeLoadDefault();

	//Load the default regardless
	CursorTypeClass::AddDefaults();

	return 0;
}