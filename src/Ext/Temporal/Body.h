#pragma once
#include <TemporalClass.h>

#include <Utilities/SavegameDef.h>

#include <Utilities/Container.h>

class WeaponTypeClass;
class TemporalExtData
{
public:	
	static COMPILETIMEEVAL size_t Canary = 0x82229781;
	 using base_type = TemporalClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };

public:

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved) { };

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(TemporalExtData) -
			(4u //AttachedToObject
				- 4u //inheritance
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class TemporalExtContainer final : public Container<TemporalExtData>
{
public:
	static TemporalExtContainer Instance;
};

class NOVTABLE FakeTemporalClass : public TemporalClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	FORCEDINLINE TemporalClass* _AsTemporal() const {
		return (TemporalClass*)this;
	}

	FORCEDINLINE TemporalExtData* _GetExtData() {
		return *reinterpret_cast<TemporalExtData**>(((DWORD)this) + AbstractExtOffset);
	}
}; 
static_assert(sizeof(FakeTemporalClass) == sizeof(TemporalClass), "Invalid Size !");
