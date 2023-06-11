#pragma once

#include <Ext/Super/Body.h>
#include <Utilities/Enum.h>

#include <array>

enum class AresNewSuperType : int
{
	SonarPulse = 0,
	UnitDelivery = 1,
	GenericWarhead = 2,
	Firestorm = 3,
	Protect = 4,
	Reveal = 5,
	ParaDrop = 6,
	SpyPlane = 7,
	ChronoSphere = 8,
	ChronoWarp = 9,
	GeneticMutator = 10,
	PsychicDominator = 11,
	LightningStorm = 12,
	NuclearMissile = 13,
	HunterSeeker = 14,
	DropPod = 15,
	EMPulse = 16,
	Battery = 17,
	EMPField = 18,
	IonCannon = 19,
	count
};

enum class AresNewActionType :int
{
	SuperWeaponDisallowed = 126,
	SuperWeaponAllowed = 127,
};

struct TargetingData
{
	TargetingData(SWTypeExt::ExtData* pTypeExt, HouseClass* pOwner) noexcept :  TypeExt(pTypeExt)
		, Owner(pOwner)
		, NeedsLaunchSite(false)
		, NeedsDesignator(false)
	{ }

	~TargetingData() noexcept = default;

	struct LaunchSite
	{
		BuildingClass* Building;
		CellStruct Center;
		double MinRange;
		double MaxRange;
	};

	struct RangedItem
	{
		int RangeSqr;
		CellStruct Center;
	};

	SWTypeExt::ExtData* TypeExt;
	HouseClass* Owner;
	bool NeedsLaunchSite;
	bool NeedsDesignator;
	std::vector<LaunchSite> LaunchSites;
	std::vector<RangedItem> Designators;
	std::vector<RangedItem> Inhibitors;
};

class NewSWType
{
	static std::vector<std::unique_ptr<NewSWType>> Array;

	static void Register(std::unique_ptr<NewSWType> pType);

	AresNewSuperType TypeIndex { AresNewSuperType(-1) };

public:

	virtual ~NewSWType() = default;

	virtual const char* GetTypeString() const { return nullptr; }
	virtual bool HandleThisType(SuperWeaponType type) const { return false; }
	virtual SuperWeaponFlags Flags() const { return SuperWeaponFlags::NoEvent; }

	virtual bool CanFireAt(TargetingData const& data, CellStruct const& cell, bool manual) const;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) = 0;
	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) { }
	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) { return false; }

	virtual void Initialize(SWTypeExt::ExtData* pData) = 0;
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) { }
	virtual bool IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const;
	virtual std::pair<double, double> GetLaunchSiteRange(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding = nullptr) const;

	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const;
	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const { return nullptr; }
	virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const { return nullptr; }
	virtual int GetSound(const SWTypeExt::ExtData* pData) const { return -1; }
	virtual int GetDamage(const SWTypeExt::ExtData* pData) const { return 0; }

public:
	// static methods
	static void Init();
	static bool IsOriginalType(SuperWeaponType nType);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static NewSWType* GetNthItem(SuperWeaponType i);
	static SuperWeaponType GetHandledType(SuperWeaponType nType);
	static NewSWType* GetNewSWType(const SWTypeExt::ExtData* pData);
	static SuperWeaponType FindFromTypeID(const char* pType);
};