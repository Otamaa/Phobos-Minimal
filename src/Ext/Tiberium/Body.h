#pragma once
#include <TiberiumClass.h>

#include <Ext/AbstractType/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

class AnimTypeClass;

class TiberiumExtData final : public AbstractTypeExtData
{
public:
	using base_type = TiberiumClass;

public:
#pragma region ClassMember

	CustomPalette Palette { CustomPalette::PaletteMode::Temperate }; //
	Nullable<AnimTypeClass*> OreTwinkle {};
	Nullable<int> OreTwinkleChance {};
	Nullable<int> Ore_TintLevel {};
	Nullable<ColorStruct> MinimapColor {};
	Valueable<bool> EnableLighningFix { true };
	Valueable<bool> UseNormalLight { true };
	Valueable<bool> EnablePixelFXAnim { true };

	Nullable<int> Damage {};
	Nullable<WarheadTypeClass*> Warhead {};

	Nullable<int> Heal_Step {};
	Nullable<int> Heal_IStep {};
	Nullable<int> Heal_UStep {};
	Nullable<double> Heal_Delay {};

	Nullable<WarheadTypeClass*> ExplosionWarhead {};
	Nullable<int> ExplosionDamage {};

	Valueable<int> DebrisChance { 33 };

	Valueable<std::string> LinkedOverlayType {};
	Valueable<int> PipIndex { -1 };
#pragma endregion
public:

	TiberiumExtData(TiberiumClass* pObj) : AbstractTypeExtData(pObj) { }
	TiberiumExtData(TiberiumClass* pObj, noinit_t& nn) : AbstractTypeExtData(pObj, nn) { }

	virtual ~TiberiumExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->AbstractTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) const
	{
		const_cast<TiberiumExtData*>(this)->AbstractTypeExtData::SaveToStream(Stm);
		const_cast<TiberiumExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->AbstractTypeExtData::CalculateCRC(crc);
	}

	virtual TiberiumClass* This() const override { return reinterpret_cast<TiberiumClass*>(this->AbstractTypeExtData::This()); }
	virtual const TiberiumClass* This_Const() const override { return reinterpret_cast<const TiberiumClass*>(this->AbstractTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { }

public:

	COMPILETIMEEVAL FORCEDINLINE AnimTypeClass* GetTwinkleAnim() const
	{
		return this->OreTwinkle.Get(RulesClass::Instance->OreTwinkle);
	}

	COMPILETIMEEVAL FORCEDINLINE int GetTwinkleChance() const
	{
		return this->OreTwinkleChance.Get(RulesClass::Instance->OreTwinkleChance);
	}

	COMPILETIMEEVAL FORCEDINLINE double GetHealDelay() const
	{
		return this->Heal_Delay.Get(RulesClass::Instance->TiberiumHeal);
	}

	int GetHealStep(TechnoClass* pTechno) const;

	COMPILETIMEEVAL FORCEDINLINE int GetDamage() const
	{
		return this->Damage.Get(MinImpl((this->AttachedToObject->Power / 10), 1));
	}

	COMPILETIMEEVAL FORCEDINLINE WarheadTypeClass* GetWarhead() const
	{
		return this->Warhead.Get(RulesClass::Instance->C4Warhead);
	}

	COMPILETIMEEVAL FORCEDINLINE WarheadTypeClass* GetExplosionWarhead() const
	{
		return this->ExplosionWarhead.Get(RulesClass::Instance->C4Warhead);
	}

	COMPILETIMEEVAL FORCEDINLINE int GetExplosionDamage() const
	{
		return this->ExplosionDamage.Get(RulesClass::Instance->TiberiumExplosionDamage);
	}

	COMPILETIMEEVAL FORCEDINLINE int GetDebrisChance() const
	{
		return this->DebrisChance;
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TiberiumExtContainer final : public Container<TiberiumExtData>
{
public:
	static TiberiumExtContainer Instance;
	static PhobosMap<OverlayTypeClass* , TiberiumClass*> LinkedType;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static void Clear();

	virtual bool WriteDataToTheByteStream(TiberiumExtData::base_type* key, IStream* pStm) { };
	virtual bool ReadDataFromTheByteStream(TiberiumExtData::base_type* key, IStream* pStm) { };
};

class NOVTABLE FakeTiberiumClass : public TiberiumClass
{
public:
#pragma region Spread
	void __RecalcSpreadData();
	void __QueueSpreadAt(CellStruct* pCell);
	void __Spread();
#pragma endregion

#pragma region Growth
	void __RecalcGrowthData();
	void __QueueGrowthAt(CellStruct* pCell);
	void __Growth();
#pragma endregion

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	TiberiumExtData* _GetExtData() {
		return *reinterpret_cast<TiberiumExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeTiberiumClass) == sizeof(TiberiumClass), "Invalid Size !");