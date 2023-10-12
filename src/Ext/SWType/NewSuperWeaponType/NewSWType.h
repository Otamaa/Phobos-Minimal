#pragma once

#include <Ext/Super/Body.h>
#include <Utilities/Enum.h>

#include <array>
#include <Utilities/VectorHelper.h>

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
	MeteorShower = 20,
	LaserStrike = 21 ,
	count
};

enum class AresNewActionType :int
{
	SuperWeaponDisallowed = 126,
	SuperWeaponAllowed = 127,
};

struct TargetingData
{
	TargetingData(SWTypeExt::ExtData* pTypeExt, HouseClass* pOwner) noexcept : TypeExt { pTypeExt }
		, Owner { pOwner }
		, NeedsLaunchSite { false}
		, NeedsDesignator { false }
		, NeedsAttractors { false }
		, NeedsSupressors { false }
		, NeedsInhibitors { false }
		, LaunchSites {}
		, Designators {}
		, Inhibitors {}
		, Attractors {}
		, Suppressors {}
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
	bool NeedsAttractors;
	bool NeedsSupressors;
	bool NeedsInhibitors;

	HelperedVector<LaunchSite> LaunchSites;
	HelperedVector<RangedItem> Designators;
	HelperedVector<RangedItem> Inhibitors;

	//Enemy Inhibitors
	HelperedVector<RangedItem> Attractors;
	//Enemy Designator
	HelperedVector<RangedItem> Suppressors;

private:
	TargetingData(const TargetingData&) = delete;
	TargetingData(TargetingData&&) = delete;
	TargetingData& operator=(const TargetingData& other) = delete;
};

class NewSWType
{
	static std::array<std::unique_ptr<NewSWType>, (size_t)AresNewSuperType::count> Array;

	static void Register(std::unique_ptr<NewSWType> pType, AresNewSuperType nType)
	{
		pType->TypeIndex = nType;
		Array[size_t(nType)] = (std::move(pType));
	}

	AresNewSuperType TypeIndex { AresNewSuperType(-1) };

public:

	NewSWType() = default;
	NewSWType(const NewSWType&) = default;
	NewSWType(NewSWType&&) = default;
	NewSWType& operator=(const NewSWType& other) = default;

	virtual ~NewSWType() = default;

	virtual std::vector<const char*> GetTypeString() const { return {}; }
	virtual bool HandleThisType(SuperWeaponType type) const { return false; }
	virtual SuperWeaponFlags Flags(const SWTypeExt::ExtData* pData) const { return SuperWeaponFlags::None; }

	virtual bool CanFireAt(TargetingData const& data, CellStruct const& cell, bool manual) const;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) = 0;
	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) { }
	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) { return false; }

	virtual void Initialize(SWTypeExt::ExtData* pData) { }
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI) { }
	virtual bool IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const;
	virtual std::pair<double, double> GetLaunchSiteRange(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding = nullptr) const;

	bool IsDesignator(const SWTypeExt::ExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;
	bool IsInhibitor(const SWTypeExt::ExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;
	bool IsAttractor(const SWTypeExt::ExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;
	bool IsSuppressor(const SWTypeExt::ExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;

	virtual SWRange GetRange(const SWTypeExt::ExtData* pData) const;
	virtual WarheadTypeClass* GetWarhead(const SWTypeExt::ExtData* pData) const;
	virtual AnimTypeClass* GetAnim(const SWTypeExt::ExtData* pData) const;
	virtual int GetSound(const SWTypeExt::ExtData* pData) const;
	virtual int GetDamage(const SWTypeExt::ExtData* pData) const;

	bool IsLaunchsiteAlive(BuildingClass* pBuilding) const;
	bool IsSWTypeAttachedToThis(const SWTypeExt::ExtData* pData ,BuildingClass* pBuilding) const;
public:

	bool HasDesignator(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool IsDesignatorEligible(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;
	bool HasInhibitor(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool IsInhibitorEligible(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;

	bool HasAttractor(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool IsAttractorEligible(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;

	bool HasSuppressor(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool IsSuppressorEligible(const SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;

	bool IsLaunchSiteEligible(SWTypeExt::ExtData* pSWType, const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const;
	bool HasLaunchSite(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords) const;

	std::unique_ptr<const TargetingData> GetTargetingData(SWTypeExt::ExtData* pData, HouseClass* pOwner) const;
	bool CanFireAt(SWTypeExt::ExtData* pData, HouseClass* pOwner, const CellStruct& cell, bool manual) const;

	TechnoClass* GetFirer(SuperClass* pSW, const CellStruct& Coords, bool ignoreRange) const;
	bool CanHaveLauchSite(SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const;

public:
	// static methods
	static void Init();
	static bool IsOriginalType(SuperWeaponType nType);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static NewSWType* GetNthItem(SuperWeaponType i);
	static SuperWeaponType GetHandledType(SuperWeaponType nType);
	static NewSWType* GetNewSWType(const SWTypeExt::ExtData* pData);
	static NewSWType* GetNewSWType(const SuperClass* pSuper);
	static SuperWeaponType FindFromTypeID(const char* pType);


};