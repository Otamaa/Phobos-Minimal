/*
	[Colors]
*/

#pragma once
#include <CRT.h>
#include <ArrayClasses.h>
#include <GeneralStructures.h>
#include <GameStrings.h>
#include <HashTable.h>

class LightConvertClass;

class ColorScheme
{
public:
	enum {
		//ColorScheme indices, since they are hardcoded all over the exe, why shan't we do it as well?
		Yellow  =   3,
		White   =   5,
		Grey    =   7,
		Red     =  11,
		Orange  =  13,
		Pink    =  15,
		Purple  =  17,
		Blue    =  21,
		Green   =  29,
	};

	//global array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<ColorScheme*>, 0xB054D0u> const Array{};
	// Player color scheme slot index to color scheme index lookup table.
	static COMPILETIMEEVAL reference<byte , 0x83ED14u, 9u> const PlayerColorToColorSchemeLUT {};

	// Game uses a hash table to store color scheme vectors for extra palettes, this table can be iterated by calling this function.
	static DynamicVectorClass<ColorScheme*>* __fastcall GetPaletteSchemesFromIterator(HashIterator* it)
	{ JMP_FAST(0x626690); }

/*
 * trap! most schemes are duplicated - ShadeCount 1 and ShadeCount 53
*/
	static NOINLINE ColorScheme* __fastcall Find(const char* pID, int ShadeCount = 1) {
		if (GameStrings::IsBlank(pID))
			return nullptr;

		for (auto pItem : *Array) {
			if (!CRT::strcmpi(pItem->ID, pID)) {
				if (pItem->ShadeCount == ShadeCount) {
					return pItem;
				}
			}
		}

		return nullptr;
	}

	static NOINLINE int __fastcall FindIndex(const char* pID, int ShadeCount = 1) {

		for(int i = 0; i < Array->Count; ++i) {

			const ColorScheme* pItem = Array->operator[](i);

			if(!CRT::strcmpi(pItem->ID, pID)) {
				if(pItem->ShadeCount == ShadeCount) {
					return i;
				}
			}
		}

		return -1;
	}

	// this does not check the `ShadeCount`
	static NOINLINE int __fastcall FindIndexById(const char* pID) {
		for (int i = 0; i < Array->Count; ++i) {
			if (!CRT::strcmpi(Array->operator[](i)->ID, pID)) {
				return i;
			}
		}

		return -1;
	}

	static ColorScheme * __fastcall FindOrAllocate(const char* pID, const ColorStruct &BaseColor, const BytePalette &Pal1, const BytePalette &Pal2, int ShadeCount)
		{ JMP_THIS(0x68C9C0); }

	static DynamicVectorClass<ColorScheme*>* __fastcall GeneratePalette(char* name)
		{ JMP_FAST(0x6263D0); }

	static int __fastcall GetNumberOfSchemes()
		{ JMP_FAST(0x626C60); }

	void CreateLightConvert(const BytePalette& Pal1, const BytePalette& Pal2, const ColorStruct& basecolor) const
		{ JMP_THIS(0x68C860); }

	bool IsSame(const ColorScheme* pColor) const
		{  JMP_THIS(0x68C960); }

	bool IsDifferent(const ColorScheme* pColor) const
		{  JMP_THIS(0x68C990); }

	//Constructor, Destructor
	ColorScheme(const char* pID, const ColorStruct &BaseColor, const BytePalette &Pal1, const BytePalette &Pal2, int ShadeCount, bool AddToArray) noexcept
		{ JMP_THIS(0x68C710); }

	ColorScheme() noexcept
		{ JMP_THIS(0x68C650); }

	~ColorScheme() noexcept
		{ JMP_THIS(0x68C8D0); }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	int                ArrayIndex; // this is off by one (always one higher than the actual index). that's because consistency and reason suck.
	DECLARE_PROPERTY(BytePalette , Colors);
	const char*       ID;
	DECLARE_PROPERTY(HSVClass, BaseColor);
	LightConvertClass* LightConvert;	//??? remap - indices #16-#31 are changed to mathefuckikally derived shades of BaseColor, think unittem.pal
	int   ShadeCount;
	PROTECTED_PROPERTY(BYTE,     unknown_314[0x1C]);
	int   MainShadeIndex;
	PROTECTED_PROPERTY(BYTE,     unknown_334[0x8]);
};

struct SchemeNode
{
	char Name[256];
	DynamicVectorClass<ColorScheme*>* Schemes;
};