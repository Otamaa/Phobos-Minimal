/*
	AbstractTypes are abstract template objects initialized by INI files
*/
#pragma once

#include <CRT.h>
#include <AbstractClass.h>
#include <Memory.h>
#include <GameStrings.h>

//forward declarations
class CCINIClass;

enum class AbstractBaseType : int
{
	Root = 0,
	ObjectType,
	TechnoType
};

#define IMPL_Find(Type)\
static NOINLINE Type##* __fastcall Find(const char* pID) {\
if (GameStrings::IsBlank(pID)) return nullptr; \
for(auto nPos = Type##::Array->begin(); nPos != Type##::Array->end(); ++nPos) \
if(CRT::strcmpi((*nPos)->ID, pID) == 0) return (*nPos); \
return nullptr; }

#define IMPL_FindByName(Type)\
static NOINLINE Type##* __fastcall FindByName(const char* pName) {\
if (GameStrings::IsBlank(pName)) return nullptr; \
for(auto nPos = Type##::Array->begin(); nPos != Type##::Array->end(); ++nPos) \
if(CRT::strcmpi((*nPos)->Name, pName) == 0) return (*nPos); \
return nullptr; }

#define IMPL_FindOrAllocate(Type)\
static NOINLINE Type##* __fastcall FindOrAllocate(const char* pID) {\
if (GameStrings::IsBlank(pID)) return nullptr; \
for(auto nPos = Type##::Array->begin(); nPos != Type##::Array->end(); ++nPos) \
if(CRT::strcmpi((*nPos)->ID, pID) == 0) return (*nPos); \
return GameCreate<Type##>(pID); }

#define IMPL_FindIndexById(Type)\
static NOINLINE int __fastcall FindIndexById(const char* pID) {\
if(Type##::Array->Count <= 0) return -1; \
for (int i = 0; i <  Type##::Array->Count; ++i)\
if (!CRT::strcmpi(Type##::Array->Items[i]->ID, pID)) return i;\
return -1;}

#define IMPL_FindIndexByName(Type)\
static NOINLINE int __fastcall FindIndexByName(const char* pName) {\
if(Type##::Array->Count <= 0) return -1; \
for (int i = 0; i < Type##::Array->Count; ++i)\
if (!CRT::strcmpi(Type##::Array->Items[i]->Name, pName)) return i; \
return -1;}

//#pragma pack(push, 4)
class NOVTABLE AbstractTypeClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Abstract;
	static const AbstractBaseType AbsTypeBase = AbstractBaseType::Root;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<AbstractTypeClass*>, 0xA8E968u> const Array {};

	IMPL_Find(AbstractTypeClass)
	IMPL_FindByName(AbstractTypeClass)
	IMPL_FindOrAllocate(AbstractTypeClass)
	IMPL_FindIndexById(AbstractTypeClass)

	//Destructor
	virtual ~AbstractTypeClass();// JMP_THIS(0x410C30);

	//AbstractTypeClass
	virtual void LoadTheaterSpecificArt(TheaterType th_type) RX;
	virtual bool LoadFromINI(CCINIClass* pINI);// JMP_THIS(0x410A60);
	virtual bool SaveToINI(CCINIClass* pINI);// JMP_THIS(0x410B90);

	const char*
		get_ID() const {
		return this->ID;
	}

	bool SameName(const char* pThat) const
		;// { JMP_THIS(0x410A40); }


	void SetName(const char* buf) const
	{
		CRT::strncpy((char *)ID, buf, sizeof(ID));
		((char &)ID[sizeof(ID) - 1]) = '\0';
	};

	//Constructor
	AbstractTypeClass(const char* pID);

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	char ID [0x18];
	BYTE zero_3C;
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