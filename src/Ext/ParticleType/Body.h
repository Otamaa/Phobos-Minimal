#pragma once
#include <ParticleTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDefB.h>

#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/PaletteManager.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

#include <Ext/ObjectType/Body.h>

class ParticleTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = ParticleTypeClass;

public:

#pragma region ClassMembers

	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types { };
	TrailsReader Trails { };
	Valueable<bool> ReadjustZ { true };
	CustomPalette Palette { CustomPalette::PaletteMode::Temperate }; //
	Valueable<double> DamageRange { 0.0 };
	Valueable<bool> DeleteWhenReachWater { false };

	std::array<Point2D, (size_t)FacingType::Count> WindMult {};

	Valueable<Point2D> Gas_DriftSpeedX { {2, -2} };
	Valueable<Point2D> Gas_DriftSpeedY { {2, -2} };

	Valueable<bool> Transmogrify { false };
	Valueable<int> TransmogrifyChance { -1 };
	Valueable<UnitTypeClass*> TransmogrifyType { nullptr };
	Valueable<OwnerHouseKind> TransmogrifyOwner { OwnerHouseKind::Neutral };

	Valueable<bool> Fire_DamagingAnim { false };
#pragma endregion

	ParticleTypeExtData(ParticleTypeClass* pObj) : ObjectTypeExtData(pObj) {
		LaserTrail_Types.reserve(2);
	}
	ParticleTypeExtData(ParticleTypeClass* pObj, noinit_t& nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~ParticleTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<ParticleTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<ParticleTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	virtual ParticleTypeClass* This() const override { return reinterpret_cast<ParticleTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const ParticleTypeClass* This_Const() const override { return reinterpret_cast<const ParticleTypeClass*>(this->ObjectTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleTypeExtContainer final : public Container<ParticleTypeExtData>
{
public:
	static ParticleTypeExtContainer Instance;

	static void Clear()
	{
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return true;
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual bool WriteDataToTheByteStream(ParticleTypeExtData::base_type* key, IStream* pStm) { return true;  };
	virtual bool ReadDataFromTheByteStream(ParticleTypeExtData::base_type* key, IStream* pStm) {  return true; };
};

class NOVTABLE FakeParticleTypeClass : public ParticleTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	bool _ReadFromINI(CCINIClass* pINI);

	ParticleTypeExtData* _GetExtData() {
		return *reinterpret_cast<ParticleTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleTypeClass) == sizeof(ParticleTypeClass), "Invalid Size !");