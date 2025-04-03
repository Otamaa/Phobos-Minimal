#pragma once
#include <InfantryClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <ExtraHeaders/CompileTimeDirStruct.h>

class InfantryExtData final
{
public:
	static COMPILETIMEEVAL size_t Canary = 0xACCAAAAA;
	using base_type = InfantryClass;
	static COMPILETIMEEVAL size_t ExtOffset = 0x6EC;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	bool IsUsingDeathSequence { false };
	int CurrentDoType { -1 };
	bool ForceFullRearmDelay { false };
	bool SkipTargetChangeResetSequence { false };

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(InfantryExtData) -
			(4u //AttachedToObject
			 );
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

class InfantryExtContainer final : public Container<InfantryExtData>
{
public:
	OPTIONALINLINE static std::vector<InfantryExtData*> Pool;
	static InfantryExtContainer Instance;

	InfantryExtData* AllocateUnchecked(InfantryClass* key) {
		InfantryExtData* val = nullptr;
		if (!Pool.empty()) {
			val = Pool.front();
			Pool.erase(Pool.begin());
		} else {
			val = DLLAllocWithoutCTOR<InfantryExtData>();
		}

		if (val) {
			//re-init
			val->InfantryExtData::InfantryExtData();
			val->AttachedToObject = key;
			return val;
		}

		return nullptr;
	}

	InfantryExtData* FindOrAllocate(InfantryClass* key)
	{
		// Find Always check for nullptr here
		if (InfantryExtData* const ptr = TryFind(key))
			return ptr;

		return this->Allocate(key);
	}

	InfantryExtData* Allocate(InfantryClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (InfantryExtData* val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	void Remove(InfantryClass* key)
	{
		if (InfantryExtData* Item = TryFind(key))
		{
			Item->~InfantryExtData();
			Item->AttachedToObject = nullptr;
			Pool.push_back(Item);
			this->ClearExtAttribute(key);
		}
	}

	void Clear()
	{
		if (!Pool.empty())
		{
			auto ptr = Pool.front();
			Pool.erase(Pool.begin());
			if (ptr)
			{
				delete ptr;
			}
		}
	}

};

class InfantryTypeExtData;
class FakeInfantryClass : public InfantryClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	void _Dummy(Mission, bool) RX;
	DamageState _IronCurtain(int nDur, HouseClass* pSource, bool bIsFC)
	{
		if (this->Type->Engineer && this->TemporalTargetingMe && this->Destination)
		{
			if (auto const pCell = this->GetCell())
			{
				if (auto const pBld = pCell->GetBuilding())
				{
					if (this->Destination == pBld && pBld->Type->BridgeRepairHut)
					{
						return DamageState::Unaffected;
					}
				}
			}
		}

		return this->TechnoClass::IronCurtain(nDur, pSource, bIsFC);
	}

	void _DestroyThis(char flag) JMP_THIS(0x523350);

	InfantryExtData* _GetExtData()
	{
		return *reinterpret_cast<InfantryExtData**>(((DWORD)this) + InfantryExtData::ExtOffset);
	}

	InfantryTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<InfantryTypeExtData**>(((DWORD)this->Type) + 0xECC);
	}
};
static_assert(sizeof(FakeInfantryClass) == sizeof(InfantryClass), "Invalid Size !");
