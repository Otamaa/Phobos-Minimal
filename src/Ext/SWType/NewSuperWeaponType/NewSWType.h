#pragma once

#include <Ext/Super/Body.h>

#include <Utilities/Enum.h>

#include <array>
#include <Utilities/VectorHelper.h>
#include <Utilities/MemoryPoolUniquePointer.h>

enum class AresNewActionType :int
{
	SuperWeaponDisallowed = 126,
	SuperWeaponAllowed = 127,
};

struct TargetingData final
{
public:

	TargetingData() : TypeExt { nullptr }
		, Owner { nullptr }
		, NeedsLaunchSite { false }
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

	TargetingData(SWTypeExtData* pTypeExt, HouseClass* pOwner) noexcept : TypeExt { pTypeExt }
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

	void reset() {
		NeedsLaunchSite = false;
		NeedsDesignator = false;
		NeedsAttractors = false;
		NeedsSupressors = false;
		NeedsInhibitors = false;
		LaunchSites.clear();
		Designators.clear();
		Inhibitors.clear();
		Attractors.clear();
		Suppressors.clear();
	}

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

	SWTypeExtData* TypeExt;
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

//private:
//	TargetingData(const TargetingData&) = delete;
//	TargetingData(TargetingData&&) = delete;
//	TargetingData& operator=(const TargetingData& other) = delete;
};

class NewSWType
{
	static std::array<std::unique_ptr<NewSWType>, (size_t)AresNewSuperType::count> Array;

	static OPTIONALINLINE
#if _HAS_CXX23 == 1
		COMPILETIMEEVAL
#endif
		void Register(std::unique_ptr<NewSWType> pType, AresNewSuperType nType)
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
	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const { return SuperWeaponFlags::None; }

	virtual bool CanFireAt(const TargetingData* pTargeting, CellStruct const& cell, bool manual) const;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) = 0;
	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) { }
	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) { return false; }

	virtual void Initialize(SWTypeExtData* pData) { }
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) { }
	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const;
	virtual std::pair<double, double> GetLaunchSiteRange(const SWTypeExtData* pData, BuildingClass* pBuilding = nullptr) const;

	bool IsDesignator(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;
	bool IsInhibitor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;
	bool IsAttractor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;
	bool IsSuppressor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;

	virtual SWRange GetRange(const SWTypeExtData* pData) const;
	virtual WarheadTypeClass* GetWarhead(const SWTypeExtData* pData) const;
	virtual AnimTypeClass* GetAnim(const SWTypeExtData* pData) const;
	virtual int GetSound(const SWTypeExtData* pData) const;
	virtual int GetDamage(const SWTypeExtData* pData) const;

	virtual void ValidateData(SWTypeExtData* pData) const { }
	bool IsLaunchsiteAlive(BuildingClass* pBuilding) const;
	bool IsSWTypeAttachedToThis(const SWTypeExtData* pData ,BuildingClass* pBuilding) const;
public:

	bool HasDesignator(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool IsDesignatorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;
	bool HasInhibitor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool IsInhibitorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;

	bool HasAttractor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool IsAttractorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;

	bool HasSuppressor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool IsSuppressorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;

	bool IsLaunchSiteEligible(SWTypeExtData* pSWType, const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const;
	bool HasLaunchSite(SWTypeExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords) const;

	void GetTargetingData(TargetingData& result , SWTypeExtData* pData, HouseClass* pOwner) const;
	bool CanFireAt(SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& cell, bool manual) const;

	TechnoClass* GetFirer(SuperClass* pSW, const CellStruct& Coords, bool ignoreRange) const;
	bool CanHaveLauchSite(SWTypeExtData* pData, BuildingClass* pBuilding) const;

public:
	// static methods
	static void Init();
	static bool IsOriginalType(SuperWeaponType nType);
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	static NewSWType* GetNthItem(SuperWeaponType i);
	static SuperWeaponType GetHandledType(SuperWeaponType nType);
	static NewSWType* GetNewSWType(const SWTypeExtData* pData);
	static NewSWType* GetNewSWType(const SuperClass* pSuper);
	static SuperWeaponType FindFromTypeID(const char* pType);


public:
	static TargetingData TargetingDataInstance;

};