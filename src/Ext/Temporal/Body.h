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
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class TemporalExtContainer final : public Container<TemporalExtData>
{
public:
	OPTIONALINLINE static std::vector<TemporalExtData*> Pool;
	static TemporalExtContainer Instance;

	TemporalExtData* AllocateUnchecked(TemporalClass* key)
	{
		TemporalExtData* val = nullptr;
		if (!Pool.empty())
		{
			val = Pool.front();
			Pool.erase(Pool.begin());
			//re-init
		}
		else
		{
			val = DLLAllocWithoutCTOR<TemporalExtData>();
		}

		if (val)
		{
			val->TemporalExtData::TemporalExtData();
			val->AttachedToObject = key;
			return val;
		}

		return nullptr;
	}

	TemporalExtData* Allocate(TemporalClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (TemporalExtData* val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	void Remove(TemporalClass* key)
	{
		if (TemporalExtData* Item = TryFind(key))
		{
			Item->~TemporalExtData();
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

class FakeTemporalClass : public TemporalClass
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
