#pragma once
#include <SuperClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

// cache all super weapon statuses
struct SWStatus
{
	bool Available; //0
	bool PowerSourced; //1
	bool Charging;

	COMPILETIMEEVAL void FORCEDINLINE reset() {
		Available = 0;
		PowerSourced = 0;
		Charging = 0;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<SWStatus*>(this)->Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Available)
			.Process(PowerSourced)
			.Process(Charging)
			.Success()
			//&& Stm.RegisterChange(this)
			; // announce this type
	}
};

class SWTypeExtData;
class SuperExtData final
{
public:
	static COMPILETIMEEVAL size_t Canary = 0x12311111;
	using base_type = SuperClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	SWTypeExtData* Type { nullptr };
	bool Temp_IsPlayer { false };
	CellStruct Temp_CellStruct { };
	bool CameoFirstClickDone { false };
	bool FirstClickAutoFireDone { false };
	SWStatus Statusses { };

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	static void UpdateSuperWeaponStatuses(HouseClass* pHouse);

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(SuperExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class SuperExtContainer final : public Container<SuperExtData>
{
public:
	static SuperExtContainer Instance;

	//CONSTEXPR_NOCOPY_CLASSB(SuperExtContainer, SuperExtData, "SuperClass");
};

class SWTypeExtData;
class NOVTABLE FakeSuperClass : public SuperClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	SuperExtData* _GetExtData() {
		return *reinterpret_cast<SuperExtData**>((DWORD)this + AbstractExtOffset);
	}

	SWTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<SWTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}
};

static_assert(sizeof(FakeSuperClass) == sizeof(SuperClass), "Invalid Size !");