#pragma once
#include <SmudgeTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SmudgeTypeExtData final
{
public:
	static COMPILETIMEEVAL size_t Canary = 0xBEE75008;
	using base_type = SmudgeTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	Valueable<bool> Clearable { true };

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(SmudgeTypeExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class SmudgeTypeExtContainer final : public Container<SmudgeTypeExtData>
{
public:
	static SmudgeTypeExtContainer Instance;

	//CONSTEXPR_NOCOPY_CLASSB(SmudgeTypeExtContainer, SmudgeTypeExtData, "SmudgeTypeClass");
};

class FakeSmudgeTypeClass : public SmudgeTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	SmudgeTypeExtData* _GetExtData() {
		return *reinterpret_cast<SmudgeTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeSmudgeTypeClass) == sizeof(SmudgeTypeClass), "Invalid Size !");
