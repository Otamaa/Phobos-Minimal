#pragma once
#include <InfantryTypeClass.h>

#include <Ext/TechnoType/Body.h>

static COMPILETIMEEVAL const char* Sequences_ident[] = {
		"Ready",
		"Guard",
		"Prone",
		"Walk",
		"FireUp",
		"Down",
		"Crawl",
		"Up",
		"FireProne",
		"Idle1",
		"Idle2",
		"Die1",
		"Die2",
		"Die3",
		"Die4",
		"Die5",
		"Tread",
		"Swim",
		"WetIdle1",
		"WetIdle2",
		"WetDie1",
		"WetDie2",
		"WetAttack",
		"Hover",
		"Fly",
		"Tumble",
		"FireFly",
		"Deploy",
		"Deployed",
		"DeployedFire",
		"DeployedIdle",
		"Undeploy",
		"Cheer",
		"Paradrop",
		"AirDeathStart",
		"AirDeathFalling",
		"AirDeathFinish",
		"Panic",
		"Shovel",
		"Carry",
		"SecondaryFire",
		"SecondaryProne",
		"SecondaryFireFly",
		"SecondaryWetAttack"
};

static COMPILETIMEEVAL std::array<DoStruct, std::size(Sequences_ident)> Sequences_Master = { {
	{1, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 0, 0, 6},
	{1, 1, 1, 3},
	{1, 0, 0, 1},
	{0, 1, 0, 1},
	{1, 1, 1, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 0, 3},
	{1, 0, 0, 3},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 3},
	{1, 1, 1, 1},
	{1, 0, 0, 3},
	{1, 0, 0, 3},
	{0, 0, 0, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 1, 0, 2},
	{1, 1, 0, 1},
	{1, 1, 0, 1},
	{1, 1, 0, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 0, 1},
	{0, 0, 0, 1},
	{0, 0, 0, 3},
	{1, 0, 0, 1},
	{0, 0, 0, 3},
	{0, 0, 0, 1},
	{0, 0, 0, 3},
	{1, 1, 1, 4},
	{1, 0, 0, 6},
	{1, 1, 1, 3},
	{1, 0, 0, 1},
	{1, 0, 0, 1},
	//{16, 0, 0, 0}, //leftover ?
	{1, 1, 0, 1}, //SecondaryFireFly
	{1, 1, 0, 1}, //SecondaryWetAttack
}
};

struct NewDoType
{
	COMPILETIMEEVAL void FORCEDINLINE Initialize()
	{
		for (auto at = this->begin(); at != this->end(); ++at)
		{
			at->StartFrame = 0;
			at->CountFrames = 0;
			at->FacingMultiplier = 0;
			at->Facing = DoTypeFacing::None;
			at->SoundCount = 0;
			at->SoundData[0].Index = -1;
			at->SoundData[0].StartFrame = 0;
			at->SoundData[1].Index = -1;
			at->SoundData[1].StartFrame = 0;
		}
	}

	COMPILETIMEEVAL FORCEDINLINE static const char* GetSequenceName(DoType sequence)
	{
		return Sequences_ident[(int)sequence];
	}

	COMPILETIMEEVAL FORCEDINLINE DoInfoStruct GetSequence(DoType sequence) const
	{
		return this->Data[(int)sequence];
	}

	COMPILETIMEEVAL FORCEDINLINE DoInfoStruct& GetSequence(DoType sequence)
	{
		return this->Data[(int)sequence];
	}

	COMPILETIMEEVAL FORCEDINLINE static DoStruct* GetSequenceData(DoType sequence)
	{
		return const_cast<DoStruct*>(&Sequences_Master[(int)sequence]);
	}

	COMPILETIMEEVAL FORCEDINLINE  DoInfoStruct* begin() { return std::begin(Data); }
	COMPILETIMEEVAL FORCEDINLINE  DoInfoStruct* end() { return std::end(Data); }

	DoInfoStruct Data[std::size(Sequences_ident)];
};

struct Phobos_DoControls
{
	static void ReadSequence(DoInfoStruct* Desig, InfantryTypeClass* pInf, CCINIClass* pINI);
};

class InfantryTypeExtData final : public TechnoTypeExtData
{
public:
	using base_type = InfantryTypeClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

#pragma region ClassMembers
	Valueable<bool> Is_Deso;
	Valueable<bool> Is_Cow;
	Nullable<double> C4Delay;
	Nullable<int> C4ROF;
	Nullable<WarheadTypeClass*> C4Warhead;
	Valueable<bool> HideWhenDeployAnimPresent;
	Valueable<bool> DeathBodies_UseDieSequenceAsIndex;
	WeaponStruct CrawlingWeaponDatas[4];
	//std::vector<DoInfoStruct> Sequences;
	ValueableIdxVector<VocClass> VoiceGarrison;
	Valueable<bool> OnlyUseLandSequences;
	std::vector<int> SquenceRates;
	// When infiltrates: detonates a weapon or damage
	Promotable<WarheadTypeClass*> WhenInfiltrate_Warhead;
	Promotable<WeaponTypeClass*> WhenInfiltrate_Weapon;
	Promotable<int> WhenInfiltrate_Damage;
	Valueable<bool> WhenInfiltrate_Warhead_Full;
	Valueable<bool> AllSequnceEqualRates;
	Valueable<bool> AllowReceiveSpeedBoost;
	Nullable<double> ProneSpeed;
	Nullable<bool> InfantryAutoDeploy;
#pragma endregion

public:
	InfantryTypeExtData(InfantryTypeClass* pObj) :
		TechnoTypeExtData(pObj),

		Is_Deso(false),
		Is_Cow(false),

		C4Delay(),
		C4ROF(),
		C4Warhead(),

		HideWhenDeployAnimPresent(false),
		DeathBodies_UseDieSequenceAsIndex(false),

		CrawlingWeaponDatas(),

		VoiceGarrison(),
		OnlyUseLandSequences(false),

		SquenceRates(),

		WhenInfiltrate_Warhead(nullptr),
		WhenInfiltrate_Weapon(nullptr),
		WhenInfiltrate_Damage(0),

		WhenInfiltrate_Warhead_Full(true),
		AllSequnceEqualRates(false),
		AllowReceiveSpeedBoost(false),

		ProneSpeed(),
		InfantryAutoDeploy()
	{
		this->AbsType = InfantryTypeClass::AbsID;
		this->InitializeConstant();
		this->Is_Deso = IS_SAME_STR_(pObj->ID, GameStrings::DESO());
		this->Is_Cow = IS_SAME_STR_(pObj->ID, GameStrings::COW());
	}

	InfantryTypeExtData(InfantryTypeClass* pObj, noinit_t nn) : TechnoTypeExtData(pObj, nn) { }

	virtual ~InfantryTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->TechnoTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->TechnoTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<InfantryTypeExtData*>(this)->TechnoTypeExtData::SaveToStream(Stm);
		const_cast<InfantryTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->TechnoTypeExtData::CalculateCRC(crc);
	}

	virtual InfantryTypeClass* This() const override { return reinterpret_cast<InfantryTypeClass*>(this->TechnoTypeExtData::This()); }
	virtual const InfantryTypeClass* This_Const() const override { return reinterpret_cast<const InfantryTypeClass*>(this->TechnoTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true;  }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class InfantryTypeExtContainer final : public Container<InfantryTypeExtData>
{
public:
	static InfantryTypeExtContainer Instance;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved) {
		for (auto& ext : Array) {
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
};

class NOVTABLE FakeInfantryTypeClass : public InfantryTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _ReadFromINI(CCINIClass* pINI);

	InfantryTypeExtData* _GetExtData() {
		return *reinterpret_cast<InfantryTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeInfantryTypeClass) == sizeof(InfantryTypeClass), "Invalid Size !");