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
	static COMPILETIMEEVAL const char* ClassName = "TiberiumExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "TiberiumClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:
#pragma region ClassMember
	CustomPalette Palette; //
	Nullable<AnimTypeClass*> OreTwinkle;
	Nullable<int> OreTwinkleChance;
	Nullable<int> Ore_TintLevel;
	Nullable<ColorStruct> MinimapColor;
	Valueable<bool> EnableLighningFix;
	Valueable<bool> UseNormalLight;
	Valueable<bool> EnablePixelFXAnim;
	Nullable<int> Damage;
	Nullable<WarheadTypeClass*> Warhead;
	Nullable<int> Heal_Step;
	Nullable<int> Heal_IStep;
	Nullable<int> Heal_UStep;
	Nullable<double> Heal_Delay;
	Nullable<WarheadTypeClass*> ExplosionWarhead;
	Nullable<int> ExplosionDamage;
	Valueable<int> DebrisChance;
	Valueable<std::string> LinkedOverlayType;
	Valueable<int> PipIndex;

	using QueueItem = std::pair<float, CellStruct>;

	struct CompareQueueItem
	{
		bool operator()(const QueueItem& a, const QueueItem& b) const
		{
			return a.first > b.first; // min-heap by float
		}
	};

	std::priority_queue<QueueItem, std::vector<QueueItem>, CompareQueueItem> SpreadQueue;
	std::vector<bool> SpreadState;
	std::priority_queue<QueueItem, std::vector<QueueItem>, CompareQueueItem> GrowthQueue;
	std::vector<bool> GrowthState;

#pragma endregion


	void Spread_AI(void);
	void Initialize_Spread(void);
	void Recalc_Spread(void);
	void Clear_Spread(void);
	void Queue_Spread(CellStruct const& cell);

	void Growth_AI(void);
	void Initialize_Growth(void);
	void Recalc_Growth(void);
	void Clear_Growth(void);
	void Queue_Growth(CellStruct const& cell);

	static void Clear_Tiberium_Spread_State(CellStruct const& cell);
	static int Map_Cell_Index(CellStruct const& cell);
	static int Map_Cell_Count();

public:

	TiberiumExtData(TiberiumClass* pObj) : AbstractTypeExtData(pObj),
		Palette(CustomPalette::PaletteMode::Temperate),
		OreTwinkle(),
		OreTwinkleChance(),
		Ore_TintLevel(),
		MinimapColor(),
		EnableLighningFix(true),
		UseNormalLight(true),
		EnablePixelFXAnim(true),
		Damage(),
		Warhead(),
		Heal_Step(),
		Heal_IStep(),
		Heal_UStep(),
		Heal_Delay(),
		ExplosionWarhead(),
		ExplosionDamage(),
		DebrisChance(33),
		LinkedOverlayType(""),
		PipIndex(-1),
		SpreadQueue(),
		SpreadState(),
		GrowthQueue(),
		GrowthState()
	{
		this->AbsType = TiberiumClass::AbsID;
	}

	TiberiumExtData(TiberiumClass* pObj, noinit_t nn) : AbstractTypeExtData(pObj, nn) { }

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

	virtual void SaveToStream(PhobosStreamWriter& Stm)
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

	TiberiumClass* This() const { return reinterpret_cast<TiberiumClass*>(this->AttachedToObject); }
	const TiberiumClass* This_Const() const { return reinterpret_cast<const TiberiumClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

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
		return this->Damage.Get(MinImpl((this->This()->Power / 10), 1));
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
				, public ReadWriteContainerInterfaces<TiberiumExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "TiberiumExtContainer";
	using base_t = Container<TiberiumExtData>;
public:

	PhobosMap<OverlayTypeClass*, TiberiumClass*> LinkedType;

public:

	static TiberiumExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);
	virtual void Clear();

	virtual void LoadFromINI(TiberiumClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(TiberiumClass* key, CCINIClass* pINI);
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

	void __Initialize_Spread() { TiberiumExtContainer::Instance.Find(this)->Initialize_Spread(); }
	void __Initialize_Growth() { TiberiumExtContainer::Instance.Find(this)->Initialize_Growth(); }
	void __Clear_Growth() { TiberiumExtContainer::Instance.Find(this)->Clear_Growth(); }
	void __Clear_Spread() { TiberiumExtContainer::Instance.Find(this)->Clear_Spread(); }

	void Clear_Tiberium_Spread_State(CellStruct const& cell)
	{
		int cellindex = TiberiumExtData::Map_Cell_Index(cell);
		TiberiumExtContainer::Instance.Find(this)->SpreadState[cellindex] = false;
	}

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	TiberiumExtData* _GetExtData() {
		return *reinterpret_cast<TiberiumExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeTiberiumClass) == sizeof(TiberiumClass), "Invalid Size !");