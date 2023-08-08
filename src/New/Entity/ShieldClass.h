#pragma once

#include <Utilities/SavegameDef.h>
#include <GeneralStructures.h>
#include <SpecificStructures.h>
#include <CoordStruct.h>
#include <RectangleStruct.h>
#include <Point2D.h>

class AbstractClass;
class ObjectClass;
class TechnoClass;
class AnimTypeClass;
class WarheadTypeClass;
class WeaponTypeClass;
class TechnoTypeClass;
class TemporalClass;
class ShieldTypeClass;
class ShieldClass final
{
public:
	ShieldClass();
	ShieldClass(TechnoClass* pTechno, bool isAttached);
	ShieldClass(TechnoClass* pTechno) : ShieldClass(pTechno, false) {};
	virtual ~ShieldClass() { KillAnim(); };

	void OnInit() { }
	void OnUnInit() { }
	void OnDetonate(CoordStruct* location) { }
	void OnPut(CoordStruct pCoord, short faceDirValue8) { }
	void OnRemove();
	void OnReceiveDamage(args_ReceiveDamage* args);
	void OnFire(AbstractClass* pTarget, int weaponIndex) { }

	void OnSelect(bool& selectable) { }
	void OnGuardCommand() { }
	void OnStopCommand() { }
	void OnDeploy() { }

	bool CanBeTargeted(WeaponTypeClass* pWeapon) const;
	bool CanBePenetrated(WarheadTypeClass* pWarhead) const;

	void OnTemporalUpdate(TemporalClass* pTemporal);
	void OnUpdate();

	void BreakShield(AnimTypeClass* pBreakAnim = nullptr, WeaponTypeClass* pBreakWeapon = nullptr);
	void SetRespawn(int duration, double amount, int rate, bool resetTimer);
	void SetSelfHealing(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer);

	void KillAnim();

	void DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound);

	double GetHealthRatio() const;
	void SetHP(int amount);
	int GetHP() const;
	bool IsActive() const;
	bool IsAvailable() const;
	bool IsBrokenAndNonRespawning() const;
	ShieldTypeClass* GetType() const;
	Armor GetArmor() const;
	int GetFramesSinceLastBroken() const;
	void SetAnimationVisibility(bool visible);

	static void SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo);
	static bool TEventIsShieldBroken(ObjectClass* pThis);

	bool IsGreenSP();
	bool IsYellowSP();
	bool IsRedSP();

	virtual void InvalidatePointer(void* ptr, bool bDetach);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);

	void UpdateType();

	void SelfHealing();
	int GetPercentageAmount(double iStatus) const;

	void RespawnShield();

	void CreateAnim();
	void UpdateIdleAnim();
	AnimTypeClass* GetIdleAnimType() const;

	void WeaponNullifyAnim(AnimTypeClass* pHitAnim = nullptr);
	void ResponseAttack() const;

	void CloakCheck();
	void OnlineCheck();
	void TemporalCheck();
	bool ConvertCheck();

	void DrawShieldBar_Building(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	void DrawShieldBar_Other(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	int DrawShieldBar_Pip(const bool isBuilding);
	int DrawShieldBar_PipAmount(int iLength);

	/// Properties ///
	TechnoClass* Techno;
	TechnoTypeClass* CurTechnoType;
	int HP;

	CDTimerClass Timers_SelfHealing_CombatRestart;
	CDTimerClass Timers_SelfHealing;
	CDTimerClass Timers_SelfHealing_Warhead;
	CDTimerClass Timers_Respawn;
	CDTimerClass Timers_Respawn_Warhead;

	AnimClass* IdleAnim;
	bool Cloak;
	bool Online;
	bool Temporal;
	bool Available;
	bool Attached;
	bool AreAnimsHidden;

	double SelfHealing_Warhead;
	int SelfHealing_Rate_Warhead;
	bool SelfHealing_RestartInCombat_Warhead;
	int SelfHealing_RestartInCombatDelay_Warhead;
	double Respawn_Warhead;
	int Respawn_Rate_Warhead;

	int LastBreakFrame;
	double LastTechnoHealthRatio;

	ShieldTypeClass* Type;
private:
	ShieldClass(const ShieldClass& other) = delete;
	ShieldClass& operator=(const ShieldClass& other) = delete;
};
