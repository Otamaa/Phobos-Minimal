/*
	AbstractTypes are abstract template objects initialized by INI files
*/
#pragma once

#include <CRT.h>
#include <AbstractClass.h>
#include <Memory.h>

//forward declarations
class CCINIClass;

enum class AbstractBaseType : int
{
	Root = 0,
	ObjectType,
	TechnoType
};

//#pragma pack(push, 4)
class NOVTABLE AbstractTypeClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Abstract;
	static const AbstractBaseType AbsTypeBase = AbstractBaseType::Root;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<AbstractTypeClass*>, 0xA8E968u> const Array {};

	static NOINLINE AbstractTypeClass* __fastcall Find(const char* pID)
	{
		for (auto pItem : *Array){ 
			if (!CRT::strcmpi(pItem->ID, pID))
				return pItem; 
		}

		return nullptr;
	}

	static NOINLINE AbstractTypeClass* __fastcall FindOrAllocate(const char* pID)
	{
		if (!pID || CRT::strcmpi(pID, GameStrings::NoneStr()) == 0 || CRT::strcmpi(pID, GameStrings::NoneStrb()) == 0)
			return nullptr;

		if (auto pRet = Find(pID)) {
			return pRet;
		}

		return GameCreate<AbstractTypeClass>(pID);
	}

	static NOINLINE int __fastcall FindIndexById(const char* pID)
	{
		if(!pID)
			return -1;

		for (int i = 0; i < Array->Count; ++i) {
			if (!CRT::strcmpi(Array->Items[i]->ID, pID)) {
				return i;
			}
		}

		return -1;
	}

	//Destructor
	virtual ~AbstractTypeClass() JMP_THIS(0x410C30);

	//AbstractTypeClass
	virtual void LoadTheaterSpecificArt(TheaterType th_type) RX;
	virtual bool LoadFromINI(CCINIClass* pINI) JMP_THIS(0x410A60);
	virtual bool SaveToINI(CCINIClass* pINI) JMP_THIS(0x410B90);

	const char* 
		get_ID() const {
		return this->ID;
	}

	bool SameName(const char* pThat) const
		{ JMP_THIS(0x410A40); }


	void SetName(const char* buf) const
	{
		CRT::strncpy((char *)ID, buf, sizeof(ID));
		((char &)ID[sizeof(ID) - 1]) = '\0';
	};

	//Constructor
	AbstractTypeClass(const char* pID) noexcept
		: AbstractTypeClass(noinit_t())
	{ JMP_THIS(0x410800); }

protected:
	explicit __forceinline AbstractTypeClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	char ID [0x18];
	PROTECTED_PROPERTY(BYTE, zero_3C);
	char UINameLabel [0x20];
	const wchar_t* UIName; //Full_Name
	char Name [0x31];

private:
	// Copy and assignment not implemented; prevent their use by declaring as private.
	AbstractTypeClass(const AbstractTypeClass&) = delete;
	AbstractTypeClass& operator=(const AbstractTypeClass&) = delete;
};
//#pragma pack(pop)
static_assert(sizeof(AbstractTypeClass) == 0x98, "Invalid size.");