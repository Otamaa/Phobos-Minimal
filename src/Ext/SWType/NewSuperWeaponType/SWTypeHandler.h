#pragma once

#include <Utilities/Enum.h>
#include <Utilities/VectorHelper.h>

#include <array>
#include <SWRange.h>
#include <CoordStruct.h>
#include <CellStruct.h>

#include <Utilities/TargetingData.h>

enum class PhobosNewActionType :int
{
	SuperWeaponDisallowed = 126,
	SuperWeaponAllowed = 127,
};

class TechnoClass;
class SuperClass;
class SWTypeExtData;
class HouseClass;
class TechnoTypeClass;
class WarheadTypeClass;
class AnimTypeClass;
class BuildingClass;
class CCINIClass;
class SWTypeHandler
{

public:

	virtual ~SWTypeHandler() = default;

	//i hate this
	virtual SuperWeaponType GetSWType() = 0;

	virtual SuperWeaponFlags Flags(const SWTypeExtData* pData) const = 0;

	virtual bool CanTargetingFireAt(const TargetingData* pTargeting, CellStruct const& cell, bool manual) const;

	virtual bool Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer) = 0;
	virtual void Deactivate(SuperClass* pThis, CellStruct cell, bool isPlayer) { }
	virtual bool AbortFire(SuperClass* pSW, bool IsPlayer) { return false; }

	virtual void Initialize(SWTypeExtData* pData) = 0;
	virtual void LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI) = 0;

	virtual WarheadTypeClass* GetWarhead(const SWTypeExtData* pData) const;
	virtual AnimTypeClass* GetAnim(const SWTypeExtData* pData) const;
	virtual int GetSound(const SWTypeExtData* pData) const;
	virtual int GetDamage(const SWTypeExtData* pData) const;
	virtual SWRange GetRange(const SWTypeExtData* pData) const;
	virtual std::pair<double, double> GetLaunchSiteRange(const SWTypeExtData* pData, BuildingClass* pBuilding) const;

	virtual bool IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const;

public:

public:

	bool IsDesignator(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;
	bool IsInhibitor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;
	bool IsAttractor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;
	bool IsSuppressor(const SWTypeExtData* pData, HouseClass* pOwner, TechnoClass* pTechno) const;

	bool IsDesignatorSimple(const SWTypeExtData* pData, HouseClass* pSWOwner, HouseClass* pTechnoOwner, TechnoTypeClass* pTechnoType) const;
	bool IsInhibitorSimple(const SWTypeExtData* pData, HouseClass* pSWOwner, HouseClass* pTechnoOwner, TechnoTypeClass* pTechnoType, bool bIsPoweredOffline) const;
	bool IsAttractorSimple(const SWTypeExtData* pData, HouseClass* pSWOwner, HouseClass* pTechnoOwner, TechnoTypeClass* pTechnoType) const;
	bool IsSuppressorSimple(const SWTypeExtData* pData, HouseClass* pSWOwner, HouseClass* pTechnoOwner, TechnoTypeClass* pTechnoType, bool bIsPoweredOffline) const;

	bool IsDesignatorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;
	bool IsInhibitorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;
	bool IsAttractorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;
	bool IsSuppressorEligible(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno) const;

	bool IsLaunchsiteAlive(BuildingClass* pBuilding) const;
	bool IsSWTypeAttachedToThis(const SWTypeExtData* pData, BuildingClass* pBuilding) const;

	bool HasDesignator(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool HasInhibitor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool HasAttractor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;
	bool HasSuppressor(const SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& Coords) const;

	bool IsLaunchSiteEligible(SWTypeExtData* pSWType, const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const;
	bool HasLaunchSite(SWTypeExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords) const;

	void PlayAnim(SuperClass* pSuper, CoordStruct& coord);

	std::unique_ptr<TargetingData> GetTargetingData(SWTypeExtData* pData, HouseClass* pOwner) const;
	bool CanFireAt(SWTypeExtData* pData, HouseClass* pOwner, const CellStruct& cell, bool manual) const;

	TechnoClass* GetFirer(SuperClass* pSW, const CellStruct& Coords, bool ignoreRange) const;
	bool CanHaveLauchSite(SWTypeExtData* pData, BuildingClass* pBuilding) const;

public:

	static SWTypeHandler* get_Handler(NewSuperType i);
	static std::pair<SuperWeaponType, NewSuperType> FindFromTypeID(const char* pType);

public:

	NewSuperType TypeIndex { NewSuperType::Invalid };
	std::vector<std::string> TypeStrings {};
};

#include <ranges>

struct TypeContainer {
	std::array<std::unique_ptr<SWTypeHandler>, (size_t)NewSuperType::count> Array;

public:

	TypeContainer();
	~TypeContainer() = default;

	static TypeContainer Instance;

private:

	void Register(std::unique_ptr<SWTypeHandler> pType, NewSuperType nType, std::string_view typeStrings);

};