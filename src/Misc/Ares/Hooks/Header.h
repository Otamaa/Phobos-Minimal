#pragma once

#include <CellStruct.h>
#include <CoordStruct.h>
#include <RectangleStruct.h>
#include <EventArgs.h>
#include <MixFileClass.h>
#include <DirStruct.h>

#include <Utilities/Enum.h>
#include <Utilities/GameUniquePointers.h>

#include <vector>
#include <New/AnonymousType/AresAttachEffectTypeClass.h>

enum class AresHijackActionResult
{
	None = 0,
	Hijack = 1,
	Drive = 2
};

enum class Persistable : unsigned int
{
	None = 0,
	unk_0x100 = 256,
	unk_0x101 = 257
};

struct PipDrawData
{
	int PipIdx;
	int DrawCount;

	PipDrawData() :PipIdx { 0 }
		, DrawCount { 1 }
	{
	}

	PipDrawData(int nIdx, int nDrawCount) :PipIdx { nIdx }, DrawCount { nDrawCount }
	{ }

	~PipDrawData() = default;
};

class CCINIClass;
struct ScriptActionNode;
class BSurface;
class ObjectClass;
class BulletClass;
class TechnoClass;
class BuildingClass;
class FootClass;
class UnitClass;
class InfantryClass;
class CaptureManagerClass;
class BuildingLightClass;
class SlaveManagerClass;
class HouseClass;
class TActionClass;
class TriggerClass;
class TEventClass;
class ParasiteClass;
class TemporalClass;
class TeamClass;
class TechnoTypeClass;
class BuildingTypeClass;
class InfantryTypeClass;
class UnitTypeClass;
class AircraftTypeClass;
class WarheadTypeClass;
class WeaponTypeClass;
class AnimTypeClass;
struct args_ReceiveDamage;
struct WeaponStruct;
class AlphaShapeClass;
class TActionClass;

struct ShakeScreenHandle
{
	static void ShakeScreen(TechnoClass* pThis, int nValToCalc, int nRules)
	{
		if (pThis->IsOnMyView())
		{
			auto nFirst = GeneralUtils::GetValue(nValToCalc);
			auto nSec = nFirst - GeneralUtils::GetValue(nRules) + 4;
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, nSec >> 1);
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, nSec);
		}
	}
};

struct AresGlobalData {

	static DWORD InternalVersion;
	static char ModName[0x40];
	static char ModVersion[0x40];
	static int ModIdentifier;
	static CSFText ModNote;
	static byte GFX_DX_Force;
	static int colorCount;
	static int version;

	static int uiColorText;
	static int uiColorTextButton; // #1644: needed for CD prompt
	static int uiColorTextCheckbox;
	static int uiColorTextRadio;
	static int uiColorTextLabel; // #1644: needed for CD prompt
	static int uiColorTextList;
	static int uiColorTextCombobox;
	static int uiColorTextGroupbox;
	static int uiColorTextEdit;
	static int uiColorTextSlider;
	static int uiColorTextObserver;
	static int uiColorCaret;
	static int uiColorSelection;
	static int uiColorSelectionCombobox;
	static int uiColorSelectionList;
	static int uiColorSelectionObserver;
	static int uiColorBorder1;
	static int uiColorBorder2;
	static int uiColorDisabled;
	static int uiColorDisabledLabel;
	static int uiColorDisabledButton;
	static int uiColorDisabledCombobox;
	static int uiColorDisabledCheckbox;
	static int uiColorDisabledList;
	static int uiColorDisabledSlider;
	static int uiColorDisabledObserver;

	struct ColorData
	{
		int colorRGB;
		int selectedIndex;
		int colorSchemeIndex;
		char colorScheme[0x20];
		const wchar_t* sttToolTipSublineText;
	};

	static ColorData Colors[16 + 1];

	static void ReadAresRA2MD(CCINIClass* Ini);
	static void ReadAresRA2MD();
};

struct StaticVars {
	static PhobosMap<ObjectClass*, AlphaShapeClass*> ObjectLinkedAlphas;
	static std::vector<unsigned char> ShpCompression1Buffer;
	static std::map<const TActionClass*, int> TriggerCounts;
	static UniqueGamePtrB<MixFileClass> aresMIX;
	static std::string MovieMDINI;
	static WaveColorData TempColor;

	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);
	static void LoadGlobalsConfig();

	static void Clear();

};

struct TechnoExt_ExtData
{
	static void AddPassengers(BuildingClass* const Grinder, TechnoClass* Vic);

	static bool IsSabotagable(BuildingClass const* const pThis);
	static Action GetiInfiltrateActionResult(InfantryClass* pInf, BuildingClass* pBuilding);
	static bool ApplyC4ToBuilding(InfantryClass* const pThis, BuildingClass* const pBuilding, const bool IsSaboteur);

	static bool IsOperated(TechnoClass* pThis);
	static bool IsOperatedB(TechnoClass* pThis);
	static bool IsPowered(TechnoClass* pThis);
	static void EvalRaidStatus(BuildingClass* pThis);
	static bool IsUnitAlive(UnitClass* pUnit);

	static void SetSpotlight(TechnoClass* pThis, BuildingLightClass* pSpotlight);
	static bool CanSelfCloakNow(TechnoClass* pThis);
	static bool IsCloakable(TechnoClass* pThis, bool allowPassive);
	static bool CloakDisallowed(TechnoClass* pThis, bool allowPassive);
	static bool CloakAllowed(TechnoClass* pThis);

	static InfantryTypeClass* GetBuildingCrew(BuildingClass* pThis, int nChance);
	static void UpdateFactoryQueues(BuildingClass const* const pBuilding);
	static bool IsBaseNormal(BuildingClass* pBuilding);

	static int GetVictimBountyValue(TechnoClass* pVictim, TechnoClass* pKiller);
	static bool KillerAllowedToEarnBounty(TechnoClass* pKiller , TechnoClass* pVictim);
	static void GiveBounty(TechnoClass* pVictim, TechnoClass* pKiller);

	static AresHijackActionResult GetActionHijack(InfantryClass* pThis, TechnoClass* const pTarget);
	static bool PerformActionHijack(TechnoClass* pFrom, TechnoClass* const pTarget);
	static bool FindAndTakeVehicle(FootClass* pThis);

	static Action GetEngineerEnterEnemyBuildingAction(BuildingClass* const pBld);

	static bool CloneBuildingEligible(BuildingClass* pBuilding, bool requirePower);
	static void KickOutClone(BuildingClass* pBuilding, TechnoTypeClass* ProductionType, HouseClass* FactoryOwner);
	static void KickOutClones(BuildingClass* pFactory, TechnoClass* const Production);

	static void InitWeapon(
		TechnoClass* pThis,
		TechnoTypeClass* pType,
		WeaponTypeClass* pWeapon,
		int idxWeapon,
		CaptureManagerClass*& pCapture,
		ParasiteClass*& pParasite,
		TemporalClass*& pTemporal,
		const char* pTagName,
		bool IsFoot
	);

	static InfantryClass* RecoverHijacker(FootClass* const pThis);
	static void SpawnSurvivors(
		FootClass* const pThis,
		TechnoClass* const pKiller,
		const bool Select,
		const bool IgnoreDefenses,
		const bool PreventPassengersEscape
	);

	static int GetWarpPerStep(TemporalClass* pThis, int nStep);
	static bool Warpable(TechnoClass* pTarget);

	static void DepositTiberium(TechnoClass* pThis , HouseClass* pHouse, float const amount, float const bonus, int const idxType);
	static void RefineTiberium(TechnoClass* pThis, HouseClass* pHouse, float const amount, int const idxType);

	static bool FiringAllowed(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon);

	static UnitTypeClass* GetUnitTypeImage(UnitClass* const pThis);
	static TechnoTypeClass* GetImage(FootClass* pThis);

	static void HandleTunnelLocoStuffs(FootClass* pOwner, bool DugIN = false, bool PlayAnim = false);

	static bool IsSameTrech(BuildingClass* currentBuilding, BuildingClass* targetBuilding);
	static bool canTraverseTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding);
	static void doTraverseTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding);

	static bool AcquireHunterSeekerTarget(TechnoClass* pThis);
	static void UpdateAlphaShape(ObjectClass* pSource);

	static int GetAmmo(TechnoClass* const pThis, WeaponTypeClass* pWeapon);
	static void DecreaseAmmo(TechnoClass* const pThis, WeaponTypeClass* pWeapon);

	static AnimClass* SpawnAnim(CoordStruct& crd, AnimTypeClass* pType, int dist);

	static void PlantBomb(TechnoClass* pSource, ObjectClass* pTarget, WeaponTypeClass* pWeapon);
	static bool CanDetonate(TechnoClass* pThis, ObjectClass* pThat);
	static Action GetAction(TechnoClass* pThis, ObjectClass* pThat);

	static int GetFirstSuperWeaponIndex(BuildingClass* pThis);
	static void UpdateDisplayTo(BuildingClass* pThis);
	static bool InfiltratedBy(BuildingClass* EnteredBuilding, HouseClass* Enterer);
	static DirStruct UnloadFacing(UnitClass* pThis);
	static CellStruct UnloadCell(BuildingClass* pThis);
	static BuildingClass* BuildingUnload(UnitClass* pThis);

	static void KickOutHospitalArmory(BuildingClass* pThis);
	static void KickOutOfRubble(BuildingClass* pBld);
	static void UpdateSensorArray(BuildingClass* pBld);
	static BuildingClass* CreateBuilding(
		BuildingClass* pBuilding,
		bool remove,
		BuildingTypeClass* pNewType,
		OwnerHouseKind owner,
		int strength,
		AnimTypeClass* pAnimType
		);

	static void Destroy(TechnoClass* pTechno, TechnoClass* pKiller, HouseClass* pKillerHouse, WarheadTypeClass* pWarhead);
	static bool IsDriverKillable(TechnoClass* pThis, double KillBelowPercent);
	static void ApplyKillDriver(TechnoClass* pTarget, TechnoClass* pKiller, HouseClass* pToOwner, bool ResetVet, Mission passiveMission);
	static bool ConvertToType(TechnoClass* pThis, TechnoTypeClass* pToType, bool AdjustHealth = true, bool IsChangeOwnership = false);

	static void RecalculateStat(TechnoClass* pThis);

	static int GetSelfHealAmount(TechnoClass* pThis);
	static void SpawnVisceroid(CoordStruct& crd, UnitTypeClass* pType, int chance, bool ignoreTibDeathToVisc , HouseClass* Owner);

	static void TransferOriginalOwner(TechnoClass* pFrom, TechnoClass* pTo);
	static void TransferIvanBomb(TechnoClass* From, TechnoClass* To);
	static void Ares_technoUpdate(TechnoClass* pThis);
	static void Ares_AddMoneyStrings(TechnoClass* pThis, bool forcedraw);
};

struct TechnoTypeExt_ExtData
{
	static bool CameoIsElite(TechnoTypeClass* pType, HouseClass* pHouse);
	static BSurface* GetPCXSurface(TechnoTypeClass* pType, HouseClass* pHouse);

	static bool CarryallCanLift(AircraftTypeClass* pCarryAll, UnitClass* Target);

	static void LoadTurrets(TechnoTypeClass* pType, CCINIClass* pINI);
	static int* GetTurretWeaponIndex(TechnoTypeClass* pType, size_t idx);
	static WeaponStruct* GetWeapon(TechnoTypeClass* pType, int const idx, bool elite);
	static void ReadWeaponStructDatas(TechnoTypeClass* pType, CCINIClass* pRules);

};

struct TechnoExperienceData
{
	static void AddAirstrikeFactor(TechnoClass*& pKiller, double& d_factor);
	static bool KillerInTransporterFactor(TechnoClass* pKiller, TechnoClass*& pExpReceiver, double& d_factor, bool& promoteImmediately);
	static void AddExperience(TechnoClass* pExtReceiver, TechnoClass* pVictim, int victimCost, double factor);
	static void MCControllerGainExperince(TechnoClass* pExpReceiver, TechnoClass* pVictim, double& d_factor, int victimCost);
	static void GetSpawnerData(TechnoClass*& pSpawnOut, TechnoClass*& pExpReceiver, double& d_spawnFacor, double& d_ExpFactor);
	static void PromoteImmedietely(TechnoClass* pExpReceiver, bool bSilent, bool Flash);
	static void UpdateVeterancy(TechnoClass*& pExpReceiver, TechnoClass* pKiller, TechnoClass* pVictim, int VictimCost, double& d_factor, bool promoteImmediately);
	static void EvaluateExtReceiverData(TechnoClass*& pExpReceiver, TechnoClass* pKiller, double& d_factor, bool& promoteImmediately);
};

struct FirewallFunctions
{
	static DWORD GetFirewallFlags(BuildingClass* pThis);
	static void ImmolateVictims(TechnoClass* pThis);
	static bool ImmolateVictim(TechnoClass* pThis, ObjectClass* const pVictim, bool const destroy = true);
	static void UpdateFirewall(BuildingClass* pThis, bool const changedState);
	static void UpdateFirewallLinks(BuildingClass* pThis);
	static bool IsActiveFirestormWall(BuildingClass* const pBuilding, HouseClass const* const pIgnore);
	static bool sameTrench(BuildingClass* currentBuilding, BuildingClass* targetBuilding);
	static bool canLinkTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding);
	static void BuildLines(BuildingClass* theBuilding, CellStruct selectedCell, HouseClass* buildingOwner);
	static int GetImageFrameIndex(BuildingClass* pThis);
};

class AresEMPulse
{
public:

	static void CreateEMPulse(WarheadTypeClass* pWarhead, const CoordStruct& Target, TechnoClass* Firer);
	static void Destroy(TechnoClass* pTechno, TechnoClass* pKiller, HouseClass* pKillerHouse, WarheadTypeClass* pWarhead);
	static AnimTypeClass* GetSparkleAnimType(TechnoClass const* const pTechno);
	static void announceAttack(TechnoClass* Techno);
	static void updateSpawnManager(TechnoClass* Techno, ObjectClass* Source = nullptr);
	static void updateRadarBlackout(BuildingClass* const pBuilding);
	static bool IsTypeEMPProne(TechnoClass* pType);
	static bool isCurrentlyEMPImmune(WarheadTypeClass* pWarhead, TechnoClass* Target, HouseClass* SourceHouse);
	static bool isEMPImmune(TechnoClass* Target, HouseClass* SourceHouse);
	static bool isEMPTypeImmune(TechnoClass* Target);
	static bool IsDeactivationAdvisable(TechnoClass* Target);
	static bool IsDeactivationAdvisableB(TechnoClass* Target);
	static void UpdateSparkleAnim(TechnoClass* pFrom, TechnoClass* pTo);
	static void UpdateSparkleAnim(TechnoClass* pWho, AnimTypeClass* pAnim = nullptr);
	static bool thresholdExceeded(TechnoClass* Victim);
	static bool isEligibleEMPTarget(TechnoClass* const pTarget, HouseClass* const pSourceHouse,WarheadTypeClass* pWarhead);
	static void deliverEMPDamage(TechnoClass* const pTechno, TechnoClass* const pFirer, WarheadTypeClass* pWarhead);
	static bool EnableEMPEffect(TechnoClass* const pVictim, ObjectClass* const pSource);
	static void DisableEMPEffect(TechnoClass* const pVictim);
	static bool EnableEMPEffect2(TechnoClass* const pVictim);
	static void DisableEMPEffect2(TechnoClass* const pVictim);
};

class AresBlitter
{
public:

	virtual ~AresBlitter() = default;
	virtual void Blit_Copy(void* dst, void* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp) = 0;
	virtual void Blit_Copy_Tinted(void* dst, void* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) = 0;
	virtual void Blit_Move(void* dst, void* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp) = 0;
	virtual void Blit_Move_Tinted(void* dst, void* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) = 0;

};

template <typename T>
class AresPcxBlit final : public AresBlitter
{
public:
	inline explicit AresPcxBlit(WORD mask) noexcept
	{
		Mask = mask;
	}

	virtual ~AresPcxBlit() override final = default;

	virtual void Blit_Copy(void* dst, void* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp) override final
	{
		auto dest = reinterpret_cast<T*>(dst);
		auto source = reinterpret_cast<T*>(src);

		if (len > 0)
		{
			for (auto i = len; i; --i)
			{
				if (auto v12 = *source++)
				{
					if (v12 != this->Mask)
						*dest = v12;
				}

				dest++;
			}
		}
	}

	virtual void Blit_Copy_Tinted(void* dst, void* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

	virtual void Blit_Move(void* dst, void* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

	virtual void Blit_Move_Tinted(void* dst, void* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

private:
	WORD Mask;
};

struct HashData
{
	DWORD Rules { 0 };
	DWORD Art { 0 };
	DWORD AI { 0 };

	static HashData GetINIChecksums();
};

struct OwnFunc
{
	static void ApplyHitAnim(ObjectClass* pTarget, args_ReceiveDamage* args);
};

struct AresScriptExt
{
	static bool Handle(TeamClass* pTeam, ScriptActionNode* pTeamMission, bool bThirdArd);

};

struct AresWPWHExt
{
	static bool conductAbduction(
		WeaponTypeClass* pWeapon,
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		CoordStruct nTargetCoords
	);

	static bool applyOccupantDamage(BulletClass* pThis);
	static void applyKillDriver(WarheadTypeClass* pWH , TechnoClass* pKiller, TechnoClass* pVictim);
};

struct AresTActionExt
{
	static std::pair<TriggerAttachType, bool> GetFlag(AresNewTriggerAction nAction);
	static std::pair<LogicNeedType, bool> GetMode(AresNewTriggerAction nAction);

	static bool ActivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);
	static bool DeactivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);
	static bool AuxiliaryPower(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);
	static bool KillDriversOf(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);
	static bool SetEVAVoice(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);
	static bool SetGroup(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);

	//TODO : re-eval
	static bool LauchhNuke(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);

	//TODO : re-eval
	static bool LauchhChemMissile(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);
	static bool LightstormStrike(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);
	static bool MeteorStrike(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);
	static bool PlayAnimAt(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);
	static bool DoExplosionAt(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);

	static bool Execute(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location, bool& ret);
};

struct AresTEventExt
{
	// Resolves a param to a house.
	static HouseClass* ResolveHouseParam(int const param, HouseClass* const pOwnerHouse = nullptr);

	// original game using between 0 - 2 ?
	// why these were 256 257 ? , something not right ,..
	static std::pair<Persistable, bool> GetPersistableFlag(AresTriggerEvents nAction);
	static std::pair<LogicNeedType, bool >  GetLogicNeed(AresTriggerEvents nAction);
	static std::pair<bool, TriggerAttachType> GetAttachFlags(AresTriggerEvents nEvent);

	static bool FindTechnoType(TEventClass* pThis, int args, HouseClass* pWho);

	// the function return is deciding if the case is handled or not
	// the bool result pointer is for the result of the Event itself
	static bool HasOccured(TEventClass* pThis, EventArgs& Args, bool& result);
};

struct TunnelFuncs
{
	static bool FindSameTunnel(BuildingClass* pTunnel);
	static void KillFootClass(FootClass* pFoot, TechnoClass* pKiller);
	static void DestroyTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, TechnoClass* pKiller);
	static void EnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pFoot);
	static bool CanEnterTunnel(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel, FootClass* pEnterer);
	static std::vector<int>* PopulatePassangerPIPData(TechnoClass* pThis, TechnoTypeClass* pType, bool& Fail);
	static std::pair<bool, FootClass*> UnlimboOne(std::vector<FootClass*>* pVector, BuildingClass* pTunnel, DWORD Where);
	static bool UnloadOnce(FootClass* pFoot, BuildingClass* pTunnel, bool silent = false);
	static void HandleUnload(std::vector<FootClass*>* pTunnelData, BuildingClass* pTunnel);
};

struct AresHouseExt
{
	static bool CheckBasePlanSanity(HouseClass* const pThis);
	static void UpdateTogglePower(HouseClass* pThis);
	static bool UpdateAnyFirestormActive(bool const lastChange);
	static void SetFirestormState(HouseClass* pHouse, bool const active);
	static void FormulateTypeList(std::vector<TechnoTypeClass*>& types, TechnoTypeClass** items, int count, int houseidx);
	static std::vector<TechnoTypeClass*> GetTypeList();
	static int GetTotalCost(const Nullable<int>& fixed);
};

struct CustomFoundation
{
	static DWORD FoundationLength(CellStruct const* const pFoundation);
	static const std::vector<CellStruct>* GetCoveredCells(BuildingClass* const pThis, CellStruct const mainCoords, int const shadowHeight);
	static void GetDisplayRect(RectangleStruct& a1, CellStruct* a2);
};

class MouseClassExt final : public MouseClass
{
public:

	static const MouseCursor* GetCursorData(MouseCursorType nMouse);
	static const MouseCursor* GetCursorDataFromRawAction(Action nAction);

#pragma region MappedAction
	struct MappedActions
	{
		size_t Idx;
		bool AllowShourd;
	};

	static std::array<MappedActions, (size_t)Action::count + 2> CursorIdx;
	static DynamicVectorClass<BuildType,DllAllocator<BuildType>> TabCameos[4u];

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		Stm.Process(CursorIdx);
		for (const auto& tab : TabCameos) {
			Savegame::WritePhobosStream(Stm, tab);
		}

		return Stm.Success();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		Stm.Process(CursorIdx) ;

		for (auto& tab : TabCameos) {
			Savegame::ReadPhobosStream(Stm, tab);
		}

		return Stm.Success();
	}

	static void ClearCameos()
	{
		for (auto& cameos : TabCameos)
		{
			cameos.Clear();
			cameos.CapacityIncrement = 100;
			cameos.Reserve(100);
		}
	}

	static void ClearMappedAction();
	static void NOINLINE InsertMappedAction(MouseCursorType nCursorIdx, Action nAction, bool Shrouded);
	static void NOINLINE InsertSWMappedAction(MouseCursorType nCursorIdx, Action nAction, bool Shrouded);
	static int NOINLINE ByWeapon(TechnoClass* pThis, int nWeaponIdx, bool OutOfRange);
#pragma endregion

public:

	//7E198C - vtable
	void _Update(const int* keyCode, const Point2D* mouseCoords);

	//7E19B0 - vtable
	bool _Override_Mouse_Shape(MouseCursorType mouse, bool wsmall = false);

	//7E19B8 - vtable
	void _Mouse_Small(bool wsmall = true);

	//5BDBC0 - Not a vtable
	int _Get_Mouse_Current_Frame(MouseCursorType mouse, bool wsmall = false) const;

	//5BDB90 - Not a vtable
	int _Get_Mouse_Frame(MouseCursorType mouse, bool wsmall = false) const;

	//5BDC00 - Not a vtable
	Point2D _Get_Mouse_Hotspot(MouseCursorType mouse) const;

	//5BE970 - Not a vtable
	int _Get_Mouse_Start_Frame(MouseCursorType mouse) const;

	//5BE990 - Not a vtable
	int _Get_Mouse_Frame_Count(MouseCursorType mouse) const;

	static size_t GetActionIndex(Action nAction);
	static MouseCursorType ValidateCursorType(Action nAction);
	static Action ValidateShroudedAction(Action nAction);
};
static_assert(sizeof(MouseClassExt) == sizeof(MouseClass), "Invalid Size !");

struct MouseCursorFuncs
{
	static void SetMouseCursorAction(size_t CursorIdx, Action nAction, bool bShrouded)
	{
#ifdef Dum
	AresData::SetMouseCursorAction(CursorIdx, nAction, bShrouded);
#else
	MouseClassExt::InsertMappedAction((MouseCursorType)CursorIdx, nAction, bShrouded);
#endif
	}
	static void SetSuperWeaponCursorAction(size_t CursorIdx, Action nAction, bool bShrouded)
	{
#ifdef Dum
	AresData::SetSWMouseCursorAction(CursorIdx, bShrouded , -1);
#else
	MouseClassExt::InsertSWMappedAction((MouseCursorType)CursorIdx, nAction, bShrouded);
#endif
	}

};