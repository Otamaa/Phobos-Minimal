#pragma once
#include <ParticleClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PooledContainer.h>
#include <Utilities/TemplateDef.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/ParticleType/Body.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

#include <Ext/Object/Body.h>

class ParticleExtData : public ObjectExtData
{
public:
	using base_type = ParticleClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:
#pragma region ClassMembers
	HelperedVector<std::unique_ptr<LaserTrailClass>> LaserTrails;
	std::vector<UniversalTrail> Trails;
#pragma endregion

public:
	ParticleExtData(ParticleClass* pObj) : ObjectExtData(pObj)
	{ }
	ParticleExtData(ParticleClass* pObj, noinit_t nn) : ObjectExtData(pObj, nn) { }

	virtual ~ParticleExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		const_cast<ParticleExtData*>(this)->ObjectExtData::SaveToStream(Stm);
		const_cast<ParticleExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectExtData::CalculateCRC(crc);
	}

	virtual ParticleClass* This() const override { return reinterpret_cast<ParticleClass*>(this->ObjectExtData::This()); }
	virtual const ParticleClass* This_Const() const override { return reinterpret_cast<const ParticleClass*>(this->ObjectExtData::This_Const()); }

public:

	static std::pair<TechnoClass*, HouseClass*> GetOwnership(ParticleClass* pThis);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleExtContainer final : public Container<ParticleExtData>
{
public:
	static ParticleExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
};

class ParticleTypeExtData;
class NOVTABLE FakeParticleClass : public ParticleClass
{
public:
	void _Detach(AbstractClass* target, bool all);

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	FORCEDINLINE ParticleClass* _AsParticle() const {
		return (ParticleClass*)this;
	}

	FORCEDINLINE ParticleExtData* _GetExtData() {
		return *reinterpret_cast<ParticleExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE ParticleTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<ParticleTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleClass) == sizeof(ParticleClass), "Invalid Size !");