#pragma once
#include <ParticleSystemTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/ObjectType/Body.h>

class ParticleSystemTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = ParticleSystemTypeClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

#pragma region ClassMembers
	Valueable<bool> ApplyOptimization;
	std::array<Point2D, (size_t)FacingType::Count> FacingMult;
	Valueable<bool> AdjustTargetCoordsOnRotation;
#pragma endregion

public:
	ParticleSystemTypeExtData(ParticleSystemTypeClass* pObj) : ObjectTypeExtData(pObj),
		ApplyOptimization(true),
		FacingMult(),
		AdjustTargetCoordsOnRotation(true)
	{ }
	ParticleSystemTypeExtData(ParticleSystemTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~ParticleSystemTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		const_cast<ParticleSystemTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<ParticleSystemTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	virtual ParticleSystemTypeClass* This() const override { return reinterpret_cast<ParticleSystemTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const ParticleSystemTypeClass* This_Const() const override { return reinterpret_cast<const ParticleSystemTypeClass*>(this->ObjectTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true;  }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleSystemTypeExtContainer final : public Container<ParticleSystemTypeExtData>
{
public:
	static ParticleSystemTypeExtContainer Instance;

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

class NOVTABLE FakeParticleSystemTypeClass : public ParticleSystemTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _ReadFromINI(CCINIClass* pINI);

	ParticleSystemTypeExtData* _GetExtData() {
		return *reinterpret_cast<ParticleSystemTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleSystemTypeClass) == sizeof(ParticleSystemTypeClass), "Invalid Size !");