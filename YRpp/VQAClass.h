#pragma once
#include <ArrayClasses.h>
#include <RectangleStruct.h>
#include <CCFileClass.h>

class BSurface;
class DSurface;

struct ALIGN(4) SubTitle
{
	DWORD Time;
	DWORD Duration;
	DWORD Color;
	DWORD YAdjust;
	DWORD XAlignment;
	wchar_t* Text;
};
static_assert(sizeof(SubTitle) == 0x18, "Invalid Size !");

struct ALIGN(4) SubTitleManager
{
	DynamicVectorClass<SubTitle>* SubTileVector;
	int currentindex;
	SubTitle* currentsub;
	void(__fastcall* SubFunction)(SubTitle*);
	BSurface* SubtitleSurface;
	RectangleStruct CurrentRect;
	RectangleStruct LastRect;
};
static_assert(sizeof(SubTitleManager) == 0x34, "Invalid Size !");

class VQAClass;
struct ALIGN(4) VQHandle
{
	static COMPILETIMEEVAL reference<VQHandle*, 0xABF3F4> const Instance {};

	static bool VQA_Get_Option(VQAAnimControlFlags)
	{	JMP_FAST(0x75A810); }

public:

	VQAClass* VQAClassPointer;
	int field_4;
	DSurface* Surface;
	DWORD dwordC_loopmode;
	DWORD dword10__loopframe;
	DWORD dword14__loopframe;
	DWORD dword18__loopframe;
	DWORD dword1C__loop_iterations;
	DWORD dword20__loopid;
	RectangleStruct InitialRect;
	RectangleStruct StretchedRect;
	char boolfield_44;
	char initdone_45;
	int SubtitlesMan;
};
static_assert(sizeof(VQHandle) == 0x4C, "Invalid Size !");