/*
	Players
*/

#pragma once
#include <Interface/IAIHouse.h>
#include <Interface/IHouse.h>
#include <Interface/IPublicHouse.h>
#include <AircraftTypeClass.h>
#include <BuildingClass.h>
#include <InfantryTypeClass.h>
#include <HouseTypeClass.h>
#include <SessionClass.h>
#include <SideClass.h>
#include <UnitClass.h>
#include <UnitTypeClass.h>
#include <ocidl.h>
#include <ScenarioClass.h>

//forward declarations
class AnimClass;
class BulletClass;
class CellClass;
class FactoryClass;
class ObjectClass;
class SuperClass;
class TagClass;
class WaypointPathClass;
class WaypointClass;
class TeamTypeClass;
class SuperWeaponTypeClass;
class UnitTrackerClass
{
public:
	static inline constexpr int Max = 0x200;

	UnitTrackerClass() JMP_THIS(0x748FD0);
	~UnitTrackerClass() = default; // JMP_THIS(0x749010);
	void IncrementUnitCount(int nUnit) JMP_THIS(0x749020);
	void DecrementUnitCount(int nUnit) JMP_THIS(0x749040);
	void PopulateUnitCount(int nCount) JMP_THIS(0x749060);
	int GetUnitCount() JMP_THIS(0x7490A0);
	const int* GetArray() JMP_THIS(0x7490C0);
	void ClearUnitCount() JMP_THIS(0x7490D0);
	void ToNetworkFormat() JMP_THIS(0x749100);
	void ToPCFormat() JMP_THIS(0x749150);

	int UnitTotals[Max];
	int UnitCount;
	BOOL InNetworkFormat;
};

struct ZoneInfoStruct
{
	int Aircraft;
	int Armor;
	int Infantry;
};

struct StartingTechnoStruct
{
	TechnoTypeClass *  Unit;
	CellStruct         Cell;
};

// that's how WW calls it, seems to track levels of how much it hates other houses... typical ww style, with bugs
struct AngerStruct
{
	HouseClass * House;
	int          AngerLevel;

	//need to define a == operator so it can be used in array classes
	bool operator == (const AngerStruct& tAnger) const
	{
		return (House == tAnger.House &&
				AngerLevel == tAnger.AngerLevel);
	}

	bool operator != (const AngerStruct& that) const
	{
		return (House != that.House && AngerLevel != that.AngerLevel);
	}
};

struct ScoutStruct
{
	HouseClass * House;
	bool         IsPreferred;

	//need to define a == operator so it can be used in array classes
	bool operator == (const ScoutStruct& tScout) const
	{
		return (House == tScout.House &&
				IsPreferred == tScout.IsPreferred);
	}

	bool operator != (const ScoutStruct& that) const
	{
		return (House != that.House && IsPreferred != that.IsPreferred);
	}
};

//--- BaseNodeClass
class BaseNodeClass
{
public:
	//need to define a == operator so it can be used in array classes
	bool operator == (const BaseNodeClass& tBaseNode) const
	{
		return
			(BuildingTypeIndex == tBaseNode.BuildingTypeIndex) &&
			(MapCoords.SimilarTo(tBaseNode.MapCoords)) &&
			(Placed == tBaseNode.Placed) &&
			(Attempts == tBaseNode.Attempts);
	}

	int        BuildingTypeIndex;
	CellStruct MapCoords;
	bool       Placed;
	int        Attempts;
};

//--- BaseClass - holds information about a player's base!
class HouseClass;	//forward declaration needed

class BaseClass
{
public:
	BaseClass()
		{ JMP_THIS(0x42E6F0); }

	BaseClass(noinit_t)
		{ JMP_THIS(0x42F1E0); }

	~BaseClass()
	{
		BaseNodes.~DynamicVectorClass();
		Cells_24.~DynamicVectorClass();
		Cells_38.~DynamicVectorClass();
	}

	//VTable
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_THIS(0x42EE30);
	virtual HRESULT __stdcall Save(IStream* pStm) JMP_THIS(0x42F070);
	virtual void ComputeCRC(CRCEngine& checksum) const JMP_THIS(0x42F180);

	//virtual ~BaseClass() { /*???*/ }; // gcc demands a virtual since virtual funcs exist

	int FailedToPlaceNode(BaseNodeClass *Node) // called after AI fails to place building, obviously
		{ JMP_THIS(0x42F380); }

	bool IsBuilt(int index)
		{ JMP_THIS(0x42E780); }

	bool IsPlaced(int a2)
		{ JMP_THIS(0x42E800); }

	BuildingClass* GetBuilding(int a2)
		{ JMP_THIS(0x42E820); }

	bool IsNode(BuildingClass* building)
		{ JMP_THIS(0x42E990); }

	BaseNodeClass* GetNode(BuildingClass* a2)
		{ JMP_THIS(0x42EA30); }

	BaseNodeClass* GetNode(CellStruct* a2)
		{ JMP_THIS(0x42EA30); }

	BaseNodeClass* NextBuildable(int type = -1)
		{ JMP_THIS(0x42EB20); }

	int NextBuildableIdx(int type) {
		JMP_THIS(0x42EB50);
	}

	void ReadINI(CCINIClass* ini, char* hname)
		{ JMP_THIS(0x42EBE0); }

	void WriteINI(CCINIClass* ini, char* hname)
		{ JMP_THIS(0x42ED60); }

	void Place_maybe(BuildingClass* building)
		{ JMP_THIS(0x42F260); }

	BaseNodeClass* Next_unplaced_maybe(BuildingTypeClass* building)
		{ JMP_THIS(0x42F340); }
//
	//Properties
	DECLARE_PROPERTY(DynamicVectorClass<BaseNodeClass>, BaseNodes);
	int                               PercentBuilt;
	DECLARE_PROPERTY(DynamicVectorClass<CellStruct>, Cells_24);
	DECLARE_PROPERTY(DynamicVectorClass<CellStruct>, Cells_38);
	CellStruct Center;

	PROTECTED_PROPERTY(BYTE,                    unknown_54[0x20]);

	HouseClass*                       Owner;

#pragma warning(suppress : 4265)
};

// used for each of the 3 drop ships. has more functions that are not reproduced here yet
struct DropshipStruct
{
	DropshipStruct() JMP_THIS(0x4B69B0);
	~DropshipStruct() JMP_THIS(0x4B69D0);

	DECLARE_PROPERTY(CDTimerClass, Timer);
	BYTE             unknown_C;
	PROTECTED_PROPERTY(BYTE, align_D[3]);
	int              Count;
	TechnoTypeClass* Types[5];
	int              TotalCost;
};

//--- Here we go, finally...
class DECLSPEC_UUID("D9D4A910-87C6-11D1-B707-00A024DDAFD1")
	NOVTABLE HouseClass : public AbstractClass, public IHouse, public IPublicHouse, public IConnectionPointContainer
{
public:
	static const AbstractType AbsID = AbstractType::House;
	static constexpr inline DWORD vtable = 0x7EA8A0;

	// <Player @ A> and friends map to these constants
	enum {PlayerAtA = 4475, PlayerAtB, PlayerAtC, PlayerAtD, PlayerAtE, PlayerAtF, PlayerAtG, PlayerAtH};

	//Static
	static inline constexpr int MaxPlayers = 8;
	static constexpr constant_ptr<DynamicVectorClass<HouseClass*>, 0xA80228u> const Array{};
	static constexpr reference<HouseClass*, 0xA83D4Cu> const CurrentPlayer{};
	static constexpr reference<HouseClass*, 0xAC1198u> const Observer{};

	//IConnectionPointContainer
	virtual HRESULT __stdcall EnumConnectionPoints(IEnumConnectionPoints** ppEnum) override R0;
	virtual HRESULT __stdcall FindConnectionPoint(const IID& riid, IConnectionPoint** ppCP) override R0;

	//IPublicHouse
	virtual long __stdcall Apparent_Category_Quantity(Category category) const override R0;
	virtual long __stdcall Apparent_Category_Power(Category category) const override R0;
	virtual CellStruct __stdcall Apparent_Base_Center() const override RT(CellStruct);
	virtual bool __stdcall Is_Powered() const override R0;

	//IHouse
	virtual LONG __stdcall ID_Number() const override R0;
	virtual BSTR __stdcall Name() const override R0;
	virtual IApplication* __stdcall Get_Application() override R0;
	virtual LONG __stdcall Available_Money() const override R0;
	virtual LONG __stdcall Available_Storage() const override R0;
	virtual LONG __stdcall Power_Output() const override R0;
	virtual LONG __stdcall Power_Drain() const override R0;
	virtual LONG __stdcall Category_Quantity(Category category) const override R0;
	virtual LONG __stdcall Category_Power(Category category) const override R0;
	virtual CellStruct __stdcall Base_Center() const override RT(CellStruct);
	virtual LONG __stdcall Fire_Sale() const override R0;
	virtual LONG __stdcall All_To_Hunt() override R0;

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override JMP_STD(0x4F6830);
	virtual ULONG __stdcall AddRef() override JMP_STD(0x50E340);
	virtual ULONG __stdcall Release() override JMP_STD(0x50E350);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x5046F0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x503040);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x504080);

	//Destructor
	virtual ~HouseClass() override JMP_THIS(0x50E380);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x4FB9B0);
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int	Size() const override R0;
	//virtual void Update() override JMP_THIS(0x4F8440);

	int GetTeamCount(TeamTypeClass* pType) const {
		JMP_THIS(0x5095D0);
	}

#ifdef _INLINED_VERSION
	bool IsAlliedWith_(int idxHouse) const
	{
		if (idxHouse == this->ArrayIndex)
		{
			return true;
		}

		if (idxHouse >= 0)
		{
			return ((1u << idxHouse) & this->Allies) != 0u;
		}
		return false;
	}

	bool IsAlliedWith_(HouseClass const* pHouse) const
	{
		return pHouse && (pHouse == this || this->IsAlliedWith_(pHouse->ArrayIndex));
	}

	bool IsAlliedWith_(ObjectClass const* pObject) const
	{
		return this->IsAlliedWith_((AbstractClass*)pObject);
	}

	bool IsAlliedWith_(TechnoClass const* pTechno) const
	{
		return this->IsAlliedWith_(pTechno->Owner);
	}

	bool IsAlliedWith_(AbstractClass const* pAbstract) const
	{
		return this->IsAlliedWith_(pAbstract->GetOwningHouse());
	}

	bool IsMutualAllies(HouseClass const* pHouse) const
	{
		return pHouse == this
			|| (this->Allies.Contains(pHouse->ArrayIndex) && pHouse->Allies.Contains(this->ArrayIndex));
	}

#endif

#pragma region NonInline
	// we use these to ensure if their functionality changes
	// we dont need to do more changes everywhere

	bool IsAlliedWith(int idxHouse) const
		{ JMP_THIS(0x4F9A10); }

	bool IsAlliedWith(HouseClass const* pHouse) const
		{ JMP_THIS(0x4F9A50); }

	bool IsAlliedWith(ObjectClass const* pObject) const
		{ JMP_THIS(0x4F9A90); }

	bool IsAlliedWith(AbstractClass const* pAbstract) const
		{ JMP_THIS(0x4F9AF0); }
#pragma endregion

	void MakeAlly(int iHouse, bool bAnnounce)
		{ JMP_THIS(0x4F9B50); }

	void MakeAlly(HouseClass* pWho, bool bAnnounce)
		{ JMP_THIS(0x4F9B70); }

	void MakeEnemy(HouseClass* pWho, bool bAnnounce)
		{ JMP_THIS(0x4F9F90); }

	void AdjustThreats()
		{ JMP_THIS(0x509400); }

	void UpdateAngerNodes(int nScoreAdd, HouseClass* pHouse)
		{ JMP_THIS(0x504790); }

	void AllyAIHouses()
		{ JMP_THIS(0x501640); }

	// no explosions, just poooof
	void SDDTORAllAndTriggers()
		{ JMP_THIS(0x4FB920); }

	void AcceptDefeat()
		{ JMP_THIS(0x4FC0B0); }

	CellStruct* WhereToGo(CellStruct* buff, TechnoClass* pDest) const {
		JMP_THIS(0x500200);
	}

	// every matching object takes damage and explodes
	void DestroyAll()
		{ JMP_THIS(0x4FC6D0); }

	void DestroyAllBuildings()
		{ JMP_THIS(0x4FC790); }

	void DestroyAllNonBuildingsNonNaval()
		{ JMP_THIS(0x4FC820); }

	void DestroyAllNonBuildingsNaval()
		{ JMP_THIS(0x4FC8D0); }

	void RespawnStartingBuildings()
		{ JMP_THIS(0x50D320); }

	void RespawnStartingForces()
		{ JMP_THIS(0x50D440); }

	BYTE Win(bool bSavourSomething)
		{ JMP_THIS(0x4FC9E0); }
	BYTE Lose(bool bSavourSomething)
		{ JMP_THIS(0x4FCBD0); }

	void RegisterJustBuilt(TechnoClass* pTechno)
		{ JMP_THIS(0x4FB6B0); }

	bool CanAlly(HouseClass* pOther) const
		{ JMP_THIS(0x501540); }

	bool CanOverpower(TechnoClass *pTarget) const
		{ JMP_THIS(0x4F9AF0); }

	// warning: logic pretty much broken
	void GainedPoweredCenter(TechnoTypeClass* pTechnoType)
	{ JMP_THIS(0x50E010); }

	void LostPoweredCenter(TechnoTypeClass* pTechnoType)
	{ JMP_THIS(0x50E0E0); }

	void HasPoweredCenter(TechnoTypeClass* pTechnoType) const
	{ JMP_THIS(0x50E1B0); }

	bool DoInfantrySelfHeal() const
		{ return this->InfantrySelfHeal > 0; }

	int GetInfSelfHealStep() const
		{ JMP_THIS(0x50D9E0); }

	bool DoUnitsSelfHeal() const
		{ return this->UnitsSelfHeal > 0; }

	int GetUnitSelfHealStep() const
		{ JMP_THIS(0x50D9F0); }

	void UpdatePower()
		{ JMP_THIS(0x508C30); }

	void CreatePowerOutage(int duration)
		{ JMP_THIS(0x50BC90); }

	double GetPowerPercentage() const
		{ JMP_THIS(0x4FCE30); }

	bool HasFullPower() const {
		return !this->PowerDrain || this->PowerOutput >= this->PowerDrain;
	}

	bool HasLowPower() const {
		return this->PowerDrain && this->PowerOutput < this->PowerDrain;
	}

	bool IsNoPower() const {
		return GetPowerPercentage() >= 1;
	}

	void CreateRadarOutage(int duration)
		{ JMP_THIS(0x50BCD0); }

	// won't work if has spysat
	void ReshroudMap()
		{ JMP_THIS(0x50BD10); }

	void Cheer()
		{ JMP_THIS(0x50C8C0); }

	void BuildingUnderAttack(BuildingClass *pBld)
		{ JMP_THIS(0x4F93E0); }

	void TakeMoney(int amount)
		{ JMP_THIS(0x4F9790); }

	void GiveMoney(int amount)
		{ JMP_THIS(0x4F9950); }

	bool CanTransactMoney(int amount) const {
		return (amount > 0) || this->Available_Money() >= -amount;
	}

	void TransactMoney(int amount) {

		if(amount > 0) {
			this->GiveMoney(amount);
		} else {
			this->TakeMoney(-amount);
		}
	}

	bool AbleToTransactMoney(int amount)
	{
		if (CanTransactMoney(amount)) {
			TransactMoney(amount);
			return true;
		}

		return false;
	}

	void GiveTiberium(float amount, int type)
		{ JMP_THIS(0x4F9610); }

	void UpdateAllSilos(int prevStorage, int prevTotalStorage)
		{ JMP_THIS(0x4F9970); }

	double GetStoragePercentage()
		{ JMP_THIS(0x4F6E70); }

	double GetWeedStoragePercentage()
		{ JMP_THIS(0x4F9750); }

	// no LostThreatNode() , this gets called also when node building dies! BUG
	void AcquiredThreatNode()
		{ JMP_THIS(0x509130); }

	// these are for mostly for map actions - HouseClass* foo = IsMP() ? Find_YesMP() : Find_NoMP();
	static bool __fastcall Index_IsMP(int idx)
		{ JMP_STD(0x510F60); }

	static HouseClass * __fastcall FindByCountryIndex(int HouseType) // find first house of this houseType
		{ JMP_STD(0x502D30); }

	static HouseClass * __fastcall FindByIndex(int idxHouse) // find house at given index
		{ JMP_STD(0x510ED0); }                    // 0..15 map to ScenarioClass::HouseIndices, also supports PlayerAtA and up

	static signed int __fastcall FindIndexByName(const char *name)
		{ JMP_STD(0x50C170); }

	static int __fastcall GetPlayerAtFromString(const char* name)
		{ JMP_STD(0x510FB0); }

	static bool __fastcall IsPlayerAtType(int at)
	{
		 JMP_STD(0x510F60);
		//return at >= PlayerAtA && at <= PlayerAtH;
	}

	static HouseClass* __fastcall FindByPlayerAt(int at)
		{ JMP_STD(0x510ED0); }

	// gets the first house of a type with this name
	static HouseClass* FindByCountryName(const char* name);

	// gets the first house of a type with name Neutral
	static HouseClass* FindNeutral();

	// gets the first house of a type with name Special
	static HouseClass* FindSpecial();

	// gets the first house of a side with this name
	static HouseClass* FindBySideIndex(int index) {
		for(auto pHouse : *Array) {
			if(pHouse->Type->SideIndex == index) {
				return pHouse;
			}
		}
		return nullptr;
	}

	constexpr int GetSpawnPosition() const {
		for (int i = 0; i < HouseClass::MaxPlayers; i++) {
			if (HouseClass::Array->GetItemOrDefault(ScenarioClass::Instance->HouseIndices[i], nullptr) == this)
				return i;
		}

		return -1;
	}


	// gets the first house of a type with this name
	static HouseClass* FindBySideName(const char* name) {
		return FindBySideIndex(SideClass::FindIndexById(name));
	}

	// gets the first house of a type from the Civilian side
	static HouseClass* FindCivilianSide();

	static void __fastcall LoadFromINIList(CCINIClass *pINI)
		{ JMP_STD(0x5009B0); }


	WaypointClass * GetPlanningWaypointAt(CellStruct *coords)
		{ JMP_THIS(0x5023B0); }

	bool GetPlanningWaypointProperties(WaypointClass *wpt, int &idxPath, BYTE &idxWP)
		{ JMP_THIS(0x502460); }

	// calls WaypointPathClass::WaypointPathClass() if needed
	void EnsurePlanningPathExists(int idx)
		{ JMP_THIS(0x504740); }

	// call after the availability of a factory has changed.
	void Update_FactoriesQueues(AbstractType factoryOf, bool isNaval, BuildCat buildCat) const
		{ JMP_THIS(0x509140); }

	// returns the factory owned by this house, having pItem in production right now, not queued
	FactoryClass* GetFactoryProducing(TechnoTypeClass const* pItem) const
		{ JMP_THIS(0x4F83C0); }

	// finds a buildingtype from the given array that this house can build
	// this checks whether the Owner=, Required/ForbiddenHouses= , AIBasePlanningSide= match and if SuperWeapon= (not SW2=) is not forbidden
	BuildingTypeClass* FirstBuildableFromArray(DynamicVectorClass<BuildingTypeClass*> const& items)
		{ JMP_THIS(0x5051E0); }

	// are all prereqs for Techno listed in vectorBuildings[0..vectorLength]. Yes, the length is needed (the vector is used for breadth-first search)
	bool AllPrerequisitesAvailable(TechnoTypeClass const* pItem, DynamicVectorClass<BuildingTypeClass*> const& vectorBuildings, int vectorLength)
		{ JMP_THIS(0x505360); }

	UnitTypeClass* PickUnitFromTypeList(const TypeList<UnitTypeClass*>& nList) const
		{ JMP_THIS(0x505310); }

	// whether any human player controls this house
	// this check if this house IsHumanPlayer or IsInPlayerControl
	// using it on wrong function can cause desyncs !
	bool IsControlledByHuman() const  { JMP_THIS(0x50B730); }

	// whether any human player controls this house
	// this check if this house IsHumanPlayer or IsInPlayerControl
	// using it on wrong function can cause desyncs !
	//bool IsControlledByHuman()_ const
	//{
	//	bool result = this->IsHumanPlayer;
	//	if(SessionClass::Instance->GameMode == GameMode::Campaign) {
	//		result |= this->IsInPlayerControl;
	//	}
	//	return result;
	//}

	// whether the human player on this PC can control this house
	// this check if this equal to HouseClass::CurrentPlayer() pointer
	// using it on wrong function can cause desyncs!
	bool ControlledByCurrentPlayer() const { JMP_THIS(0x50B6F0); }

	// whether the human player on this PC can control this house
	// this check if this equal to HouseClass::CurrentPlayer() pointer
	// using it on wrong function can cause desyncs!
	//bool ControlledByCurrentPlayer_() const
	//{
	//	if(SessionClass::Instance->GameMode != GameMode::Campaign) {
	//		return this->IsCurrentPlayer();
	//	}
	//	return this->IsHumanPlayer || this->IsInPlayerControl;
	//}

	// Target ought to be Object, I imagine, but cell doesn't work then
	void __fastcall SendSpyPlanes(int AircraftTypeIdx, int AircraftAmount, Mission SetMission, AbstractClass *Target, ObjectClass *Destination)
		{ JMP_STD(0x65EAB0); }

	// registering in prereq counters (all technoes get logged, but only buildings get checked on validation... wtf)
	void RegisterGain(TechnoClass* pTechno, bool ownerChange)
		{ JMP_THIS(0x502A80); }

	void RegisterLoss(TechnoClass* pTechno, bool keepTiberium)
		{ JMP_THIS(0x5025F0); }

	BuildingClass* FindBuildingOfType(int idx, int sector = -1) const
		{ JMP_THIS(0x4FD060); }

	static AnimClass * __fastcall PsiWarn(HouseClass* pOwner , CellClass *pTarget, BulletClass *Bullet, char *AnimName)
		JMP_STD(0x43B5E0);

	bool Fire_LightningStorm(SuperClass* pSuper)
		{ JMP_THIS(0x509E00); }

	bool Fire_ParaDrop(SuperClass* pSuper)
		{ JMP_THIS(0x509CD0); }

	bool Fire_PsyDom(SuperClass* pSuper)
		{ JMP_THIS(0x50A150); }

	bool Fire_GenMutator(SuperClass* pSuper)
		{ JMP_THIS(0x509F60); }

	bool IonSensitivesShouldBeOffline() const
		{ JMP_THIS(0x53A130); }

	const char *get_ID() const {
		return this->Type->get_ID();
	}

	int FindSuperWeaponIndex(SuperWeaponType type) const;

	SuperClass* FindSuperWeapon(SuperWeaponType type) const;
	SuperClass* FindSuperWeapon(SuperWeaponTypeClass* pType) const;

	// I don't want to talk about these
	// read the code <_<

	void AddCounters_OwnedNow(TechnoClass const* const pItem)
	{ JMP_THIS(0x4FF700); }

	void SubCounters_OwnedNow(TechnoClass const* const pItem)
	{ JMP_THIS(0x4FF550); }

	//  Count owned now
	constexpr int CountOwnedNow(TechnoTypeClass const* pItem) const
	{
		switch (VTable::Get(pItem))
		{
		case BuildingTypeClass::vtable:
			return this->CountOwnedNow(
				static_cast<BuildingTypeClass const*>(pItem));
		case UnitTypeClass::vtable:
			return this->CountOwnedNow(
				static_cast<UnitTypeClass const*>(pItem));
		case InfantryTypeClass::vtable:
			return this->CountOwnedNow(
				static_cast<InfantryTypeClass const*>(pItem));
		case AircraftTypeClass::vtable:
			return this->CountOwnedNow(
				static_cast<AircraftTypeClass const*>(pItem));
		default:
			return 0;
		}
	}
	constexpr int CountOwnedNow(BuildingTypeClass const* const pItem) const {
		return this->OwnedBuildingTypes.GetItemCount(pItem->ArrayIndex);
	}

	constexpr int CountOwnedNow(AircraftTypeClass const* const pItem) const {
		return this->OwnedAircraftTypes.GetItemCount(pItem->ArrayIndex);
	}

	constexpr int CountOwnedNow(InfantryTypeClass const* const pItem) const {
		return this->OwnedInfantryTypes.GetItemCount(pItem->ArrayIndex);
	}

	constexpr int CountOwnedNow(UnitTypeClass const* const pItem) const {
		return this->OwnedUnitTypes.GetItemCount(pItem->ArrayIndex);
	}

	// RegisterGain
	void AddCounters_OwnedPresent(const TechnoClass* pItem , bool ownerChange)
	{ JMP_THIS(0x502A80); }

	// RegisterLoss
	void SubCounters_OwnedPresent(const TechnoClass* pItem , bool keepTiberium)
	{ JMP_THIS(0x5025F0); }

	// Count owned and present
	constexpr int CountOwnedAndPresent(TechnoTypeClass* pItem) const
	{
		switch (VTable::Get(pItem))
		{
		case BuildingTypeClass::vtable:
			return this->CountOwnedAndPresent((BuildingTypeClass*)pItem);
		case UnitTypeClass::vtable:
			return this->CountOwnedAndPresent((UnitTypeClass*)pItem);
		case InfantryTypeClass::vtable:
			return this->CountOwnedAndPresent((InfantryTypeClass*)pItem);
		case AircraftTypeClass::vtable:
			return this->CountOwnedAndPresent((AircraftTypeClass*)pItem);
		default:
			return 0;
		}
	}

	constexpr int CountOwnedAndPresent(BuildingTypeClass* pItem) const {
		return this->ActiveBuildingTypes.GetItemCount(pItem->ArrayIndex);
	}

	constexpr int CountOwnedAndPresent(AircraftTypeClass* pItem) const {
		return this->ActiveAircraftTypes.GetItemCount(pItem->ArrayIndex);
	}

	constexpr int CountOwnedAndPresent(InfantryTypeClass* pItem) const {
		return this->ActiveInfantryTypes.GetItemCount(pItem->ArrayIndex);
	}

	constexpr int CountOwnedAndPresent(UnitTypeClass* pItem) const {
		return this->ActiveUnitTypes.GetItemCount(pItem->ArrayIndex);
	}

	void AddCounters_OwnedEver(TechnoClass const* const pItem)
	{ JMP_THIS(0x4FB6B0); }

	// Count owned ever
	constexpr int CountOwnedEver(TechnoTypeClass* pItem) const
	{
		switch (VTable::Get(pItem))
		{
		case BuildingTypeClass::vtable:
			return this->CountOwnedEver((BuildingTypeClass*)pItem);
		case UnitTypeClass::vtable:
			return this->CountOwnedEver((UnitTypeClass*)pItem);
		case InfantryTypeClass::vtable:
			return this->CountOwnedEver((InfantryTypeClass*)pItem);
		case AircraftTypeClass::vtable:
			return this->CountOwnedEver((AircraftTypeClass*)pItem);
		default:
			return 0;
		}
	}

	constexpr int CountOwnedEver(BuildingTypeClass* pItem) const {
		return this->FactoryProducedBuildingTypes.GetItemCount(pItem->ArrayIndex);
	}

	constexpr int CountOwnedEver(AircraftTypeClass* pItem) const {
		return this->FactoryProducedAircraftTypes.GetItemCount(pItem->ArrayIndex);
	}

	constexpr int CountOwnedEver(InfantryTypeClass* pItem) const {
		return this->FactoryProducedInfantryTypes.GetItemCount(pItem->ArrayIndex);
	}

	constexpr int CountOwnedEver(UnitTypeClass* pItem) const {
		return this->FactoryProducedUnitTypes.GetItemCount(pItem->ArrayIndex);
	}

	bool HasFromSecretLab(TechnoTypeClass* pItem) const {
		for(const auto& pLab : this->SecretLabs) {
			if(pLab->GetSecretProduction() == pItem) {
				return true;
			}
		}
		return false;
	}

	constexpr bool HasAllStolenTech(TechnoTypeClass* pItem) const {
		if(pItem->RequiresStolenAlliedTech && !this->Side0TechInfiltrated) { return false; }
		if(pItem->RequiresStolenSovietTech && !this->Side1TechInfiltrated) { return false; }
		if(pItem->RequiresStolenThirdTech && !this->Side2TechInfiltrated) { return false; }
		return true;
	}

	bool HasFactoryForObject(TechnoTypeClass* pItem) const {
		const auto abs = pItem->WhatAmI();
		const auto naval = pItem->Naval;
		for(auto const& pBld : this->Buildings) {
			auto pType = pBld->Type;
			if(pType->Factory == abs && pType->Naval == naval) {
				return true;
			}
		}
		return false;
	}

	bool CanExpectToBuild(const TechnoTypeClass* pItem) const;

	bool CanExpectToBuild(const TechnoTypeClass* pItem, int idxParent) const;

	bool InOwners(const TechnoTypeClass* const pItem) const {
		auto const idxParentCountry = this->Type->FindParentCountryIndex();
		return pItem->InOwners(1u << idxParentCountry);
	}

	bool InRequiredHouses(const TechnoTypeClass* const pItem) const {
		return pItem->InRequiredHouses(1u << this->Type->ArrayIndex2);
	}

	bool InForbiddenHouses(const TechnoTypeClass* const pItem) const {
		return pItem->InForbiddenHouses(1u << this->Type->ArrayIndex2);
	}

	CanBuildResult CanBuild(TechnoTypeClass const* pItem, bool buildLimitOnly, bool allowIfInProduction) const
		{ JMP_THIS(0x4F7870); }

	int AI_BaseConstructionUpdate()
		{ JMP_THIS(0x4FE3E0); }

	int AI_VehicleConstructionUpdate()
		{ JMP_THIS(0x4FEA60); }

	void AI_TryFireSW() const
		{ JMP_THIS(0x5098F0); }

	bool Fire_SW(int idx, const CellStruct &coords)
		{ JMP_THIS(0x4FAE50); }

	CellStruct* PickTargetByType(CellStruct &outBuffer, TargetType targetType) const
		{ JMP_THIS(0x50D170); }

	CellStruct PickTargetByType(TargetType targetType) const {
		CellStruct outBuffer;
		this->PickTargetByType(outBuffer, targetType);
		return outBuffer;
	}

	CellStruct* PickIonCannonTarget(CellStruct &outBuffer) const
		{ JMP_THIS(0x50CBF0); }

	CellStruct PickIonCannonTarget() const {
		CellStruct outBuffer;
		this->PickIonCannonTarget(outBuffer);
		return outBuffer;
	}

	bool IsIonCannonEligibleTarget(const TechnoClass* pTechno) const;

	void UpdateFlagCoords(UnitClass *NewCarrier, DWORD dwUnk)
		{ JMP_THIS(0x4FBE40); }

	void DroppedFlag(CellStruct *Where, UnitClass *Who)
		{ JMP_THIS(0x4FBF60); }

	bool PickedUpFlag(UnitClass *pWho, DWORD dwUnk)
	{ JMP_THIS(0x4FC060); }

	FactoryClass* GetPrimaryFactory(AbstractType absID, bool naval, BuildCat buildCat) const
		{ JMP_THIS(0x500510); }

	void SetPrimaryFactory(FactoryClass* pFactory, AbstractType absID, bool naval, BuildCat buildCat)
		{ JMP_THIS(0x500850); }

	constexpr const CellStruct& GetBaseCenter() const {
		if(this->BaseCenter.IsValid()) {
			return this->BaseCenter;
		} else {
			return this->BaseSpawnCell;
		}
	}

	void SetBaseSpawnCell(const CellStruct& place) const {
		JMP_THIS(0x50E000);
	}

	void SetBaseSpawnCell(const CellStruct& place)  {
		this->BaseSpawnCell = place;
	}

	constexpr unsigned int FORCEINLINE GetAIDifficultyIndex() const {
		return static_cast<unsigned int>(this->AIDifficulty);
	}

	constexpr unsigned int GetCorrectAIDifficultyIndex() const
	{
		switch (AIDifficulty)
		{
		case AIDifficulty::Hard:
			return 2;
		case AIDifficulty::Normal:
			return 1;
		default:
			return 0;
		}
	}

	/*!
		At the moment, this function is really just a more intuitively named mask for
		this->Type->MultiplayPassive, but it might be expanded into something more
		complicated later.

		Primarily used to check if something is owned by the neutral house.
		\return true if house is passive in multiplayer, false if not.
		\author Renegade
		\date 01.03.10
	*/
	constexpr bool IsNeutral() const {
		return this->Type->MultiplayPassive;
	}

	// whether this house is equal to Player
	constexpr bool IsCurrentPlayer() const {
		return this == CurrentPlayer;
	}

	//ControlledByCurrentPlayer
	//bool IsControlledByCurrentPlayer() const { JMP_THIS(0x50B730); }
	//{
	//	bool result = CurrentPlayer;
	//	if (SessionClass::Instance->GameMode == GameMode::Campaign) {
	//		result = result || IsInPlayerControl;
	//	}
	//
	//	return result;
	//}

	// whether this house is equal to Observer
	constexpr bool IsObserver() const {

		return (this == Observer //|| !CRT::strcmpi(get_ID(), "Observer")
			);

	}

	// whether Player is equal to Observer
	constexpr static bool IsCurrentPlayerObserver() {
		return CurrentPlayer && CurrentPlayer->IsObserver();
	}

	int CalculateCostMultipliers()
		{ JMP_THIS(0x50BF60); }

	void WhimpOnMoney(AbstractType nAbsType)
		{ JMP_THIS(0x509700); }

	void RemoveTracking(TechnoClass* pTechno)
		{ JMP_THIS(0x4FF550); }

	void AddTracking(TechnoClass* pTechno)
	{ JMP_THIS(0x4FF700); }

	constexpr Edge GetHouseEdge() const
	{
		auto edge = this->StaticData.StartingEdge;
		if (edge < Edge::North || edge > Edge::West)
		{
			edge = this->Edge;

			if (edge < Edge::North || edge > Edge::West)
			{
				edge = Edge::North;
			}
		}

		return edge;
	}

	Edge GetCurrentEdge() const {
		JMP_THIS(0x50DA80);
	}

	Edge ResolveEdge() const
		{ JMP_THIS(0x50DAC0); }

	Edge GetStartingEdge() const
		{ JMP_THIS(0x50DAA0); }

	double GetBuildTimeMult(TechnoTypeClass* pThat)
		{ JMP_THIS(0x50C0A0); }

	double GetTypeArmorMult(TechnoTypeClass* pTech)
		{ JMP_THIS(0x50BD30); }

	void GiveWeed(int nAmount ,int nIndex) const
		{ JMP_THIS(0x4F9700); }

	AbstractClass* FindTargetOnCoords(CoordStruct& nCoord)
		{ JMP_THIS(0x500300); }

	int FactoryCount(AbstractType nWhat, bool IsNaval)
		{ JMP_THIS(0x500910); }

	void ForceEnd()
		{ JMP_THIS(0x4FCDC0); }

	void UpdateSuperWeaponsOwned() const
		{ JMP_THIS(0x50AF10); }

	void UpdateSuperWeaponsUnavailable() const
		{ JMP_THIS(0x50B1D0); }

	void UpdateScoutNodes(HouseClass* house) const
		{ JMP_THIS(0x504860); }

	CoordStruct* GetBaseCenter(CoordStruct* ret) const
		{ JMP_THIS(0x50DF30); }

	void basecenter_4FAF00(SuperClass* a2,CellStruct& nCell) const
		{ JMP_THIS(0x4FAF00); }

	void Func_505180() const
		{ JMP_THIS(0x505180); }

	bool GrandOpening(bool Captured) const
		{ JMP_THIS(0x445F80); }

	double GetHouseTypeCostMult(TechnoTypeClass* pType) const
		{ JMP_THIS(0x50BDF0); }

	double GetHouseCostMult(TechnoTypeClass* pType) const
		{ JMP_THIS(0x50BEB0); }

	CellStruct* RandomCellInZone(CellStruct* pResult, ZoneType zone) const {
		JMP_THIS(0x501AC0);
	}

	CellStruct RandomCellInZone(ZoneType zone) const
	{
		CellStruct nBuffer;
		RandomCellInZone(&nBuffer, zone);
		return nBuffer;
	}

	double GetSpeedMult(TechnoTypeClass* pWho) const {
		JMP_THIS(0x50C050);
	}

	AbandonProductionResult AbandonProduction(AbstractType rtti ,int index ,bool naval ,bool AbandonAll) const {
		JMP_THIS(0x4FAA10);
	}

	constexpr bool MakeObserver() const
	{
		if (HouseClass::CurrentPlayer != this)
			return false;

		HouseClass::Observer = const_cast<HouseClass*>(this);
		return true;
	}

	constexpr bool inline IsInitiallyObserver() const
	{
		return this->IsHumanPlayer && (this->GetSpawnPosition() == -1);
	}

	bool HasSpaceFor(BuildingTypeClass* pBld , CellStruct* where) const {
		JMP_THIS(0x50B760);
	}

	typedef int(__fastcall* placement_callback)(int , int);
	CellStruct* FindBuildLocation(CellStruct* buffer, BuildingTypeClass* pBld, placement_callback, int something) const {
		JMP_THIS(0x5060B0);
	}

	CellStruct* GetPoweups(CellStruct* buffer, BuildingTypeClass* a3) const {
		JMP_THIS(0x506B90);
	}

	//Constructor
	HouseClass(HouseTypeClass* pCountry) noexcept
		: HouseClass(noinit_t())
	{ JMP_THIS(0x4F54A0); }

protected:
	explicit __forceinline HouseClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int                   ArrayIndex;
	HouseTypeClass*       Type;
	DECLARE_PROPERTY(DynamicVectorClass<TagClass*>, RelatedTags);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, ConYards);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, Buildings);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, UnitRepairStations);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, Grinders);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, Absorbers);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, Bunkers);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, Occupiables);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, CloningVats);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, SecretLabs);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, PsychicDetectionBuildings);
	DECLARE_PROPERTY(DynamicVectorClass<BuildingClass*>, FactoryPlants);
	int                   CountResourceGatherers;
	int                   CountResourceDestinations;
	int                   CountWarfactories;
	int                   InfantrySelfHeal;
	int                   UnitsSelfHeal;
	DECLARE_PROPERTY(DynamicVectorClass<StartingTechnoStruct*>, StartingUnits);
	AIDifficulty          AIDifficulty; // be advised that it's reverse, Hard == 0 and Easy == 2. I'm sure Westwood has a good reason for this. Yep.
	double                FirepowerMultiplier; // used
	double                GroundspeedMultiplier; // unused ...
	double                AirspeedMultiplier;
	double                ArmorMultiplier;
	double                ROFMultiplier;
	double                CostMultiplier;
	double                BuildTimeMultiplier; // ... unused ends
	double                RepairDelay;
	double                BuildDelay;
	struct StaticClass {
		int                   IQLevel;
		int                   TechLevel;
		IndexBitfield<HouseClass*> AltAllies;        // ask question, receive brain damage
		int                   StartingCredits;       // not sure how these are used // actual credits = this * 100
		Edge                  StartingEdge;
	}StaticData;
	DWORD                 AIState_1E4;
	int                   SideIndex;
	bool                  IsHumanPlayer;	// 1EC	,is controlled by the player at this computer ,IsHuman
	bool                  IsInPlayerControl;		// 1ED , a human controls this House
	bool                  Production;		//AI production has begun
	bool                  AutocreateAllowed;
	bool			      NodeLogic_1F0;
	bool			      ShipYardConst_1F1;
	bool                  AITriggersActive;
	bool                  AutoBaseBuilding;
	bool                  DiscoveredByPlayer;
	bool                  Defeated;
	bool                  IsGameOver;
	bool                  IsWinner;
	bool                  IsLoser;
	bool                  CiviliansEvacuated; // used by the CivEvac triggers
	bool                  FirestormActive;
	bool                  HasThreatNode;
	bool                  RecheckTechTree;
	int					  IPAddress;
	int					  TournamentTeamID;
	bool				  LostConnection;
	int                   SelectedPathIndex;
	WaypointPathClass*    PlanningPaths [12];    // 12 paths for "planning mode"
	char                  Visionary;             //??? exe says so
	bool                  MapIsClear;
	bool                  IsTiberiumShort;
	bool                  HasBeenSpied;
	bool                  HasBeenThieved; // Something of this house has been entered by a Thief/VehicleThief
	bool                  Repairing; // BuildingClass::Repair, handholder for hurr durf AI
	bool                  IsBuiltSomething;
	bool                  IsResigner;
	bool                  IsGiverUpper;
	bool                  AllToHunt;
	bool                  IsParanoid;
	bool                  IsToLook;
	int                   IQLevel2;			//no idea why we got this twice
	AIMode                AIMode;
	DECLARE_PROPERTY(DynamicVectorClass<SuperClass*>, Supers);
	int                   LastBuiltBuildingType;
	int                   LastBuiltInfantryType;
	int                   LastBuiltAircraftType;
	int                   LastBuiltVehicleType;
	int                   AllowWinBlocks;        // some ra1 residue map trigger-fu, should die a painful death
	DECLARE_PROPERTY(CDTimerClass, RepairTimer); // for AI
	DECLARE_PROPERTY(CDTimerClass, AlertTimer);
	DECLARE_PROPERTY(CDTimerClass, BorrowedTime);
	DECLARE_PROPERTY(CDTimerClass, PowerBlackoutTimer);
	DECLARE_PROPERTY(CDTimerClass, RadarBlackoutTimer);
	bool                  Side2TechInfiltrated;  // asswards! whether this player has infiltrated stuff
	bool                  Side1TechInfiltrated;  // which is listed in [AI]->BuildTech
	bool                  Side0TechInfiltrated;  // and has the appropriate AIBasePlanningSide
	bool                  BarracksInfiltrated;
	bool                  WarFactoryInfiltrated;

		// these four are unused horrors
		// checking prerequisites:
		/*
		if(1 << this->Country->IndexInArray & item->RequiredHouses
			|| (item->WhatAmI == abs_InfantryType && (item->RequiredHouses & this->InfantryAltOwner))
			|| (item->WhatAmI == abs_UnitType && (item->RequiredHouses & this->UnitAltOwner))
			|| (item->WhatAmI == abs_AircraftType && (item->RequiredHouses & this->AircraftAltOwner))
			|| (item->WhatAmI == abs_BuildingType && (item->RequiredHouses & this->BuildingAltOwner))
		)
			{ can build }
		 */
	DWORD InfantryAltOwner;
	DWORD UnitAltOwner;
	DWORD AircraftAltOwner;
	DWORD BuildingAltOwner;

	int                   AirportDocks;
	int                   PoweredUnitCenters;
	int					  CreditsSpent;
	int					  HarvestedCredits;
	int					  StolenBuildingsCredits;
	int                   OwnedUnits;
	int                   OwnedNavy;
	int                   OwnedBuildings;
	int                   OwnedInfantry;
	int                   OwnedAircraft;
	BYTE				  OwnedTiberium[sizeof(StorageClass)];
	int                   Balance;
	int                   TotalStorage; // capacity of all building Storage
	DECLARE_PROPERTY(StorageClass, OwnedWeed);
	DWORD unknown_324;
	DECLARE_PROPERTY(UnitTrackerClass, BuiltAircraftTypes);
	DECLARE_PROPERTY(UnitTrackerClass, BuiltInfantryTypes);
	DECLARE_PROPERTY(UnitTrackerClass, BuiltUnitTypes);
	DECLARE_PROPERTY(UnitTrackerClass, BuiltBuildingTypes);
	DECLARE_PROPERTY(UnitTrackerClass, KilledAircraftTypes);
	DECLARE_PROPERTY(UnitTrackerClass, KilledInfantryTypes);
	DECLARE_PROPERTY(UnitTrackerClass, KilledUnitTypes);
	DECLARE_PROPERTY(UnitTrackerClass, KilledBuildingTypes);
	DECLARE_PROPERTY(UnitTrackerClass, CapturedBuildings);
	DECLARE_PROPERTY(UnitTrackerClass, CollectedCrates);	//YES, THIS IS HOW WW WASTES TONS OF RAM
	int                   NumAirpads;
	int                   NumBarracks;
	int                   NumWarFactories;
	int                   NumConYards;
	int                   NumShipyards;
	int                   NumOrePurifiers;
	float                 CostInfantryMult;
	float                 CostUnitsMult;
	float                 CostAircraftMult;
	float                 CostBuildingsMult;
	float                 CostDefensesMult;
	int                   PowerOutput;
	int                   PowerDrain;
	FactoryClass*         Primary_ForAircraft;
	FactoryClass*         Primary_ForInfantry;
	FactoryClass*         Primary_ForVehicles;
	FactoryClass*         Primary_ForShips;
	FactoryClass*         Primary_ForBuildings;
	FactoryClass*         Primary_Unused1;
	FactoryClass*         Primary_Unused2;
	FactoryClass*         Primary_Unused3;
	FactoryClass*         Primary_ForDefenses;
	BYTE				  AircraftType_53D0;
	BYTE				  InfantryType_53D1;
	BYTE				  VehicleType_53D2;
	BYTE				  ShipType_53D3;
	BYTE				  BuildingType_53D4;
	BYTE				  unknown_53D5;
	BYTE				  unknown_53D6;
	BYTE				  unknown_53D7;
	BYTE				  DefenseType_53D8;
	BYTE				  unknown_53D9;
	BYTE				  unknown_53DA;
	BYTE				  unknown_53DB;
	UnitClass*			  OurFlagCarrier;
	CellStruct			  OurFlagCoords;
	//for endgame score screen
	int                   KilledUnitsOfHouses [20];     // 20 Houses only!
	int                   TotalKilledUnits;
	int                   KilledBuildingsOfHouses [20]; // 20 Houses only!
	int                   TotalKilledBuildings;
	int                   WhoLastHurtMe;
	CellStruct            BaseSpawnCell;
	CellStruct            BaseCenter; // set by map action 137 and 138
	int                   Radius;
	DECLARE_PROPERTY_ARRAY(ZoneInfoStruct, ZoneInfos, 5);
	int                   LATime;
	int                   LAEnemy;
	BuildingClass*                   ToCapture;
//	IndexBitfield<HouseTypeClass *> RadarVisibleTo; // these house types(!?!, fuck you WW) can see my radar
	IndexBitfield<HouseClass *> RadarVisibleTo;  // this crap is being rewritten to use house indices instead of house types
	int                   SiloMoney;
	TargetType			  PreferredTargetType; // Set via map action 35. The preferred object type to attack.
	CellStruct			  PreferredTargetCell; // Set via map action 135 and 136. Used to override firing location of targettable SWs.
	CellStruct			  PreferredDefensiveCell; // Set via map action 140 and 141, or when an AIDefendAgainst SW is launched.
	CellStruct			  PreferredDefensiveCell2; // No known function sets this to a real value, but it would take precedence over the other.
	int					  PreferredDefensiveCellStartTime; // The frame the PreferredDefensiveCell was set. Used to fire the Force Shield.

		// Used for: Counting objects ever owned
		// altered on each object's loss or gain
		// BuildLimit > 0 validation uses this
		//	XQuantity on topsonIDB
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, OwnedBuildingTypes);
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, OwnedUnitTypes);
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, OwnedInfantryTypes);
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, OwnedAircraftTypes);

		// Used for: Counting objects currently owned and on the map
		// altered on each object's loss or gain
		// AITriggerType condition uses this
		// original PrereqOverride check uses this
		// original Prerequisite check uses this
		// AuxBuilding check uses this
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, ActiveBuildingTypes);
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, ActiveUnitTypes);
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, ActiveInfantryTypes);
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, ActiveAircraftTypes);

		// Used for: Counting objects produced from Factory
		// not altered when things get taken over or removed
		// BuildLimit < 0 validation uses this
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, FactoryProducedBuildingTypes);
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, FactoryProducedUnitTypes);
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, FactoryProducedInfantryTypes);
	DECLARE_PROPERTY(CounterClass<GameAllocator<int>>, FactoryProducedAircraftTypes);

	DECLARE_PROPERTY(CDTimerClass, AttackTimer);
	int                   InitialAttackDelay; // both unused
	int                   EnemyHouseIndex;
	DECLARE_PROPERTY(DynamicVectorClass<AngerStruct>, AngerNodes); //arghghghgh bugged
	DECLARE_PROPERTY(DynamicVectorClass<ScoutStruct>, ScoutNodes); // filled with data which is never used, jood gob WW
	DECLARE_PROPERTY(CDTimerClass, AITimer);
	DECLARE_PROPERTY(CDTimerClass, ExpertAITimer);
	int                   ProducingBuildingTypeIndex;
	int                   ProducingUnitTypeIndex;
	int                   ProducingInfantryTypeIndex;
	int                   ProducingAircraftTypeIndex;
	int                   RatioAITriggerTeam;
	int                   RatioTeamAircraft;
	int                   RatioTeamInfantry;
	int                   RatioTeamBuildings;
	int                   BaseDefenseTeamCount;
	DECLARE_PROPERTY_ARRAY(DropshipStruct, DropshipData, 3);
	int                   CurrentDropshipIndex;
	byte				  HasCloakingRanges; // don't ask
	ColorStruct			  Color;
	ColorStruct			  LaserColor; // my idb says so
	BaseClass			  Base;
	bool                  RecheckPower;
	bool                  RecheckRadar;
	bool                  SpySatActive;
	bool                  IsBeingDrained;
	Edge                  Edge;
	CellStruct            EMPTarget;
	CellStruct            NukeTarget;
	IndexBitfield<HouseClass*> Allies; // flags, one bit per HouseClass instance
	                                   //-> 32 players possible here
	DECLARE_PROPERTY(CDTimerClass, DamageDelayTimer);
	DECLARE_PROPERTY(CDTimerClass, TeamDelayTimer); // for AI attacks
	DECLARE_PROPERTY(CDTimerClass, TriggerDelayTimer);
	DECLARE_PROPERTY(CDTimerClass, SpeakAttackDelayTimer);
	DECLARE_PROPERTY(CDTimerClass, SpeakPowerDelayTimer);
	DECLARE_PROPERTY(CDTimerClass, SpeakMoneyDelayTimer);
	DECLARE_PROPERTY(CDTimerClass, SpeakMaxedDelayTimer);
	IAIHouse*             AIGeneral;

	unsigned int          ThreatPosedEstimates[130][130]; // BLARGH

	char				  PlainName[21]; // this defaults to the owner country's name in SP or <human player><computer player> in MP. Used as owner for preplaced map objects
	char				  UINameString[33]; // this contains the UIName= text from the INI! or
	wchar_t				  UIName [21]; // this contains the CSF string from UIName= above, or a copy of the country's UIName if not defined. Take note that this is shorter than the country's UIName can be...
	int                   ColorSchemeIndex;
	union{
		int                   StartingPoint;
		CellStruct            StartingCell;
	};
	IndexBitfield<HouseClass*> StartingAllies;
	DWORD                 unknown_16060;
	DECLARE_PROPERTY(DynamicVectorClass<IConnectionPoint*>, WaypointPath);
	DWORD __ConnectionPoints_1607C;
	DWORD unknown_16080;
	DWORD unknown_16084; //unused , can be used to store ExtData
	double unused_16088;
	double unused_16090;
	DWORD padding_16098;
	float PredictionEnemyArmor; // defaults to 0.33, AIForcePredictionFudge'd later
	float PredictionEnemyAir;
	float PredictionEnemyInfantry;
	int TotalOwnedInfantryCost;
	int TotalOwnedVehicleCost;
	int TotalOwnedAircraftCost;
	DWORD unknown_power_160B4;
};

static_assert(sizeof(HouseClass) == 0x160B8, "Invalid Size !.");