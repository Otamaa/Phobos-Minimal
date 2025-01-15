#pragma once

#include <YRPPCore.h>
#include <CRT.h>
#include <GenericList.h>
#include <ArrayClasses.h>
#include <CCFileClass.h>
#include <IndexClass.h>
#include <Helpers/CompileTime.h>
#include <PKey.h>
#include <TRect.h>
#include <CoordStruct.h>
#include <ColorStruct.h>
#include <GameStrings.h>
#include <Helpers/VTable.h>

class TechnoTypeClass;

//Basic INI class
class INIClass
{
public:
	struct INIComment
	{
		char* Value;
		INIComment* Next;
	};
	static_assert(sizeof(INIComment) == 0x8, "Invalid size.");

	class INIEntry : public Node<INIEntry>
	{
	public:
		char* Key;
		char* Value;
		INIComment* Comments;
		char* CommentString;
		int PreIndentCursor;
		int PostIndentCursor;
		int CommentCursor;
	};
	static_assert(sizeof(INIEntry) == 0x28, "Invalid size.");

	class INISection : public Node<INISection>
	{
	public:
		using EntryIndexes = IndexClass<int, INIEntry*>;

		char* Name;
		List<INIEntry*> Entries;
		EntryIndexes EntryIndex;
		INIComment* Comments;

		void DeallocINISection(){
			JMP_THIS(0x52AB80);
		}

		void VectorDealloc(char args) {
			JMP_THIS(0x52AE00);
		}

		//virtual ~INISection() = default;
		//INISection() = delete; //TODO
	};
	static_assert(sizeof(INISection) == 0x44, "Invalid size.");


public:
	using IndexType = IndexClass<int, INISection*>;

	char* CurrentSectionName;
	INISection* CurrentSection;
	DECLARE_PROPERTY(List<INISection>, Sections);
	DECLARE_PROPERTY(IndexType, SectionIndex); // <CRCValue of the Name, Pointer to the section>
	INIComment* LineComments;

public:
	INIClass()
		{ JMP_THIS(0x535AA0); }

protected:
	INIClass(bool) { }

public:

	virtual ~INIClass();

	void Reset()
		{ JMP_THIS(0x526B00); }

	void Clear(const char *s1, char *s2)
		{ JMP_THIS(0x5257C0); }

	INISection* GetSection(const char* pSection)
		{ JMP_THIS(0x526810); }

	INIEntry* GetKey(const char* pSection , const char* pKey)
		{ JMP_THIS(0x526B10); }

	int GetKeyCount(const char* pSection) //Get the amount of keys in a section.
		{ JMP_THIS(0x526960); }

	const char* GetKeyName(const char* pSection, int nKeyIndex) //Get the name of a key number in a section.
		{ JMP_THIS(0x526CC0); }

	bool Is_Loaded() const { return !Sections.IsEmpty(); }

	bool Is_Present(const char *pSection, const char *pKey = nullptr)
	{
		if (pKey == nullptr)
			return GetSection(pSection);

		return GetKey(pSection, pKey) != nullptr;
	}

	//Reads an ANSI string. Returns the string's length.
	int ReadString(const char* pSection, const char* pKey, const char* pDefault, char* pBuffer, size_t szBufferSize)
		{ JMP_THIS(0x528A10); }

	int GetString(const char* pSection, const char* pKey, char* pBuffer,size_t szBufferSize)
		{ return ReadString(pSection, pKey, "", pBuffer ,szBufferSize); }

	//Writes an ANSI string.
	bool WriteString(const char* pSection, const char* pKey, const char* pString)
		{ JMP_THIS(0x528660); }

	//Reads an escaped Unicode string. Returns the string's length.
	int ReadUnicodeString(const char* pSection, const char* pKey, const wchar_t* pDefault, wchar_t* pBuffer, size_t szBufferSize)
		{ JMP_THIS(0x528F00); }

	//Writes an escaped Unicode string.
	bool WriteUnicodeString(const char* pSection, const char* pKey, const wchar_t* pString)
		{ JMP_THIS(0x528E00); }

	//Reads an boolean value.
	bool ReadBool(const char* pSection, const char* pKey, bool bDefault)
		{ JMP_THIS(0x5295F0); }

	void GetBool(const char* pSection, const char* pKey, bool& bValue)
		{ bValue = ReadBool(pSection, pKey, bValue); }

	//Writes an boolean value.
	bool WriteBool(const char* pSection, const char* pKey, bool bValue)
		{ JMP_THIS(0x529560); }

	//Reads an integer value.
	int ReadInteger(const char* pSection, const char* pKey, int nDefault)
		{ JMP_THIS(0x5276D0); }

	void GetInteger(const char* pSection, const char* pKey, int& nValue)
		{ nValue = ReadInteger(pSection, pKey, nValue); }

	OPTIONALINLINE void GetIntegerClamp(const char *pSection, const char *pKey, int nMin, int nMax, int& nValue)
		{nValue = std::clamp(ReadInteger(pSection, pKey, nValue), nMin, nMax);}

	//Writes an integer value.
	bool WriteInteger(const char* pSection, const char* pKey, int nValue, bool bHex)
		{ JMP_THIS(0x5275C0); }

	//Reads a decimal value.
	double ReadDouble(const char* pSection, const char* pKey, double dDefault)
	{
		double* pdDefault = &dDefault;
		PUSH_VAR64(pdDefault); PUSH_VAR32(pKey); PUSH_VAR32(pSection); THISCALL(0x5283D0);
		_asm {fstp dDefault};
		return dDefault;
	}

	void GetDouble(const char* pSection, const char* pKey, double& nValue)
		{ nValue = ReadDouble(pSection, pKey, nValue); }

	OPTIONALINLINE void GetDoubleClamp(const char* pSection, const char* pKey, double nMin, double nMax ,double& nValue)
		{ nValue = std::clamp(ReadDouble(pSection, pKey, nValue), nMin, nMax); }

	//Writes a decimal value.
	bool WriteDouble(const char* pSection, const char* pKey, double dValue)
	{
		double* pdValue=&dValue;
		PUSH_VAR64(pdValue); PUSH_VAR32(pKey); PUSH_VAR32(pSection); THISCALL(0x5285B0);
	}

	int ReadRate(const char* pSection, const char* pKey, int nDefault)
	{
		double buffer = nDefault / 900.0;
		GetDouble(pSection, pKey, buffer);
		return static_cast<int>(buffer * 900.0);
	}

	void GetRate(const char* pSection, const char* pKey, int& nValue)
		{ nValue = ReadRate(pSection, pKey, nValue); }

	bool WriteRate(const char* pSection, const char* pKey, int nValue)
	{
		double dValue = nValue / 900.0;
		return WriteDouble(pSection, pKey, dValue);
	}

	float ReadFloat(const char *pSection, const char *pKey, float fDefault)
		{ return (float)ReadDouble(pSection, pKey, fDefault); }

	OPTIONALINLINE void GetFloatClamp(const char *pSection, const char *pKey, float nMin, float nMax, float& fDefault)
		{ fDefault = std::clamp(ReadFloat(pSection, pKey, fDefault), nMin, nMax); }

	bool WriteFloat(const char *pSection, const char *pKeys, float fVal)
		{ return WriteDouble(pSection, pKeys, fVal); }

	//Reads two integer values.
	int* Read2Integers(int* pBuffer, const char* pSection, const char* pKey, int* pDefault)
		{ JMP_THIS(0x529880); }

	Point2D* ReadPoint2D(Point2D& ret, const char* pSection, const char* pKey, Point2D& defValue)
		{ JMP_THIS(0x529880); }

	void GetPoint2D(const char* pSection, const char* pKey, Point2D& value)
		{ ReadPoint2D(value, pSection, pKey, value); }

	//Writes two integer values.
	bool Write2Integers(const char* pSection, const char* pKey, int* pValues)
		{ JMP_THIS(0x5297E0); }

	//Reads three integer values.
	int* Read3Integers(int* pBuffer, const char* pSection, const char* pKey, int* pDefault)
		{ JMP_THIS(0x529CA0); }

	CoordStruct* ReadCoords(CoordStruct* pCoord , const char* pSection, const char* pKey, CoordStruct pDefault)
		{ JMP_THIS(0x476420); }

	CoordStruct* ReadPoint3D(CoordStruct& ret, const char* pSection, const char* pKey, CoordStruct& defValue)
		{ JMP_THIS(0x529CA0); }
	void GetPoint3D(const char* pSection, const char* pKey, CoordStruct& value)
		{ ReadPoint3D(value, pSection, pKey, value); }

	//Reads three byte values.
	byte* Read3Bytes(byte* pBuffer, const char* pSection, const char* pKey, byte* pDefault)
		{ JMP_THIS(0x474B50); }

	//Writes three byte values.
	bool Write3Bytes(const char* pSection, const char* pKey, byte* pValues)
		{ JMP_THIS(0x474C20); }

	//Tests whether the given section and key exists. If key is NULL, only the section will be looked for.
	bool Exists(const char* pSection, const char* pKey)
		{ JMP_THIS(0x679F40); }

	_GUID* ReadUID(_GUID* result ,const char* pSection , const char* pKey, _GUID* pDefault)
		{ JMP_THIS(0x527920); }

	bool PutUID(const char* pSection, const char* pKey, _GUID* pValue)
		{ JMP_THIS(0x527B90); }

	TRect<int>* GetRect(TRect<int> *result, const char* pSection, const char* entry, TRect<int>* pDefault)
		{ JMP_THIS(0x527CC0); }

	PKey* GetPkey(PKey* pResult, bool bFast)
		{ JMP_THIS(0x52A670); }

	bool PutPkey(PKey* pKey)
		{ JMP_THIS(0x52A610); }

	// C&C helpers

#define INI_READ(item, addr) \
	int Read ## item(const char* pSection, const char* pKey, int pDefault) \
		{ JMP_THIS(addr); }

#define INI_READ_DETERMINED(item, addr) \
	item Read ## item(const char* pSection, const char* pKey, item pDefault) \
		{ JMP_THIS(addr); }

	// Pip= to idx ( pip strings with index < pDefault are not even scanned! )
	INI_READ(Pip, 0x4748A0);

	// PipScale= to idx
	INI_READ_DETERMINED(PipScale, 0x474940);

	// Category= to idx
	INI_READ_DETERMINED(Category, 0x4749E0);

	// Color=%s to idx
	INI_READ(ColorString, 0x474A90);

	// Foundation= to idx
	INI_READ_DETERMINED(Foundation, 0x474DA0);

	// MovementZone= to idx
	INI_READ_DETERMINED(MovementZone, 0x474E40);

	// SpeedType= to idx
	INI_READ_DETERMINED(SpeedType, 0x476FC0);

	// [SW]Action= to idx
	INI_READ(SWAction, 0x474EE0);

	// [SW]Type= to idx
	INI_READ(SWType, 0x474F50);

	// EVA Event name to idx
	INI_READ(VoxName, 0x474FA0);

	// Factory= to idx
	INI_READ(Factory, 0x474FF0);

	INI_READ_DETERMINED(BuildCat, 0x475060);

	// Parses a list of Countries and returns a bitfield, i.e. Owner= or RequiredHouses=
	INI_READ(HouseTypesList, 0x4750D0);

	// Parses a list of Houses and returns a bitfield, i.e. Allies= in map
	INI_READ(HousesList, 0x475260);

	INI_READ(ArmorType, 0x4753F0);

	INI_READ_DETERMINED(LandType, 0x4754B0);

	// supports MP names (<Player @ X>) too, wtf
	// ALLOCATES if country name is not found
	// returns idx of country it reads
	INI_READ(HouseType, 0x475540);

	// ALLOCATES if name is not found
	INI_READ(Side, 0x4756F0);

	// returns index of movie with this filename
	INI_READ(Movie, 0x4757D0);

	// map theater
	INI_READ(Theater, 0x475870);

	INI_READ(Theme, 0x4758F0);

	INI_READ_DETERMINED(Edge, 0x475980);

	INI_READ_DETERMINED(Powerup, 0x4759F0);

	// [Anim]Layer= to idx
	INI_READ_DETERMINED(Layer, 0x477050);

	INI_READ(VHPScan, 0x477590);

	// Color=%d,%d,%d to idx , used to parse [Colors]
	ColorStruct* ReadColor(ColorStruct* pBuffer, const char* pSection, const char* pKey, ColorStruct const& defValue)
		{ JMP_THIS(0x474C70); }

	ColorStruct ReadColor(const char* const pSection, const char* const pKey, ColorStruct const& defValue) {
		ColorStruct outBuffer;
		this->ReadColor(&outBuffer, pSection, pKey, defValue);
		return outBuffer;
	}

	void GetColor(const char* const pSection, const char* const pKey, ColorStruct& value)
	{
		ReadColor(&value, pSection, pKey, value);
	}

	bool WriteColor(const char* const pSection, const char* const pKey, ColorStruct const& color)
		{ JMP_THIS(0x474D50); }

	// OverlayPack, OverlayDataPack, IsoMapPack5
	// Those uses 1=xxxx, 2=xxxx, 3=xxxx .etc.
	size_t ReadUUBlock(const char* const pSection, void* pBuffer, size_t length)
		{ JMP_THIS(0x526FB0); }

	bool WriteUUBlock(const char* const pSection, void* pBuffer, size_t length)
		{ JMP_THIS(0x526E80); }

	// 18 bytes
	byte* ReadAbilities(byte* pBuffer, const char* pSection, const char* pKey, byte* pDefault)
		{ JMP_THIS(0x477640); }


	TechnoTypeClass* ReadTechnoType(const char* pSection, const char* pKey)
		{ JMP_THIS(0x476EB0); }

	// safer and more convenient overload for string reading
	template <size_t Size>
	COMPILETIMEEVAL int ReadString(const char* pSection, const char* pKey, const char* pDefault, char(&pBuffer)[Size])
	{
		return this->ReadString(pSection, pKey, pDefault, pBuffer, Size);
	}

	template <size_t Size>
	COMPILETIMEEVAL int GetString(const char* pSection, const char* pKey, char(&pBuffer)[Size])
	{
		return ReadString(pSection, pKey, pBuffer, pBuffer);
	}

	// safer and more convenient overload for escaped unicode string reading
	template <size_t Size>
	COMPILETIMEEVAL int ReadUnicodeString(const char* pSection, const char* pKey, const wchar_t* pDefault, wchar_t(&pBuffer)[Size])
	{
		return this->ReadUnicodeString(pSection, pKey, pDefault, pBuffer, Size);
	}


	// fsldargh who the fuck decided to pass structures by value here
	static TypeList<int>* __fastcall GetPrerequisites(TypeList<int>* pBuffer, INIClass* pINI,
		const char* pSection, const char* pKey, TypeList<int> ndefaults)
			{ JMP_STD(0x4770E0); }

	TypeList<int>* GetTypeList(TypeList<int>* ret , const char* pSection , const char* pKey , TypeList<int> def)
		{ JMP_THIS(0x475D70); }

	int ReadTime(const char* pSection, const char* pKey, int nDefault)
		{ JMP_THIS(0x52A760); }

	bool WriteTime(const char* pSection, const char* pKey, int nValue)
		{ JMP_THIS(0x52A940); }
	//Properties
};
static_assert(sizeof(INIClass) == 0x40);//64

class AircraftTypeClass;
class InfantryTypeClass;
class WarheadTypeClass;
//Extended INI class specified for C&C use
class CCINIClass : public INIClass
{
public:
	//STATIC
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7E1AF4;

	static COMPILETIMEEVAL reference<DWORD, 0xB77E00u> const RulesHash{};
	static COMPILETIMEEVAL reference<DWORD, 0xB77E04u> const ArtHash{};
	static COMPILETIMEEVAL reference<DWORD, 0xB77E08u> const AIHash{};

	static COMPILETIMEEVAL reference<DWORD, 0xAC026Cu> const RulesHash_Internet{};
	static COMPILETIMEEVAL reference<DWORD, 0xAC0270u> const ArtHash_Internet{};
	static COMPILETIMEEVAL reference<DWORD, 0xAC0274u> const AIHash_Internet{};

	// westwood genius shines again

	// this is a pointer in the class
	static COMPILETIMEEVAL reference<CCINIClass*, 0x887048u> const INI_Rules{};

	// these are static class variables, why the fuck did you differentiate them, WW?
	static COMPILETIMEEVAL reference<CCINIClass, 0x887128u> const INI_AI{};
	static COMPILETIMEEVAL reference<CCINIClass, 0x887180u> const INI_Art{};
	static COMPILETIMEEVAL reference<CCINIClass, 0x887208u> const INI_UIMD{};
	static COMPILETIMEEVAL reference<CCINIClass, 0x8870C0u> const INI_RA2MD{};

	//non-static
	CCINIClass() : INIClass(false) {
		THISCALL(0x535AA0);
		Digested = false;
		VTable::Set(this, vtable);
	}

	virtual ~CCINIClass() = default;

	static CCINIClass* LoadINIFile(const char* pFileName)
	{
		CCINIClass* pINI = GameCreate<CCINIClass>();
		if(CCFileClass* pFile = GameCreate<CCFileClass>(pFileName)){
			if (pFile->Exists())
				pINI->ReadCCFile(pFile);

			GameDelete<true,false>(pFile);
		}
		return pINI;
	}

	static void UnloadINIFile(CCINIClass*& pINI)
	{
		if (pINI) {
			GameDelete<true,false>(std::exchange(pINI , nullptr));
		}
	}

	//Parses an INI file from a CCFile
	CCINIClass* ReadCCFile(FileClass* pCCFile, bool bDigest = false, bool bLoadComments = false)
		{ JMP_THIS(0x4741F0); }

	void WriteCCFile(FileClass *pCCFile, bool bDigest = false)
		{ JMP_THIS(0x474430); }

	//Copies the string table entry pointed to by the INI value into pBuffer.
	int ReadStringtableEntry(const char* pSection, const char* pKey, wchar_t* pBuffer, size_t szBufferSize)
		{ JMP_THIS(0x0529160); }

	template <size_t Size>
	int ReadStringtableEntry(const char* pSection, const char* pKey, wchar_t(&pBuffer)[Size])
	{
		return this->ReadStringtableEntry(pSection, pKey, pBuffer, Size);
	}

	template<class T>
	TypeList<T*> Get_TypeList(const char* section, const char* entry, const TypeList<T*> defvalue, const DynamicVectorClass<T*>& heap);

	template<class T>
	bool Put_TypeList(const char* section, const char* entry, const TypeList<T*> value);

	DWORD GetCRC()
		{ JMP_THIS(0x476D80); }

	//Most of them are inlined
#define FINDORMAKETYPE(C,addr) C* ##C##_FindOrMake(const char* pSection, const char* pKey , C* pDefault) { JMP_THIS(addr); }

	FINDORMAKETYPE(AircraftTypeClass, 0x67BD30u)
	FINDORMAKETYPE(InfantryTypeClass, 0x67BAC0u)
	FINDORMAKETYPE(WarheadTypeClass , 0x67B500u)
#undef FINDORMAKETYPE

	//Properties

public:

	bool Digested : 1;
	byte Digest[20];
};
static_assert(sizeof(CCINIClass) == 0x58, "Invalid size.");//85
