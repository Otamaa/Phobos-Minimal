#pragma once

#include <GeneralStructures.h>
#include <SpecificStructures.h>

#include <Ext/TechnoType/Body.h>

class TechnoClass;
class WarheadTypeClass;

class ShieldClass final
{
public:
	ShieldClass();
	ShieldClass(TechnoClass* pTechno, bool isAttached);
	ShieldClass(TechnoClass* pTechno) : ShieldClass(pTechno, false) {};
	virtual ~ShieldClass() { KillAnim(); };

	virtual void OnInit() { }
	virtual void OnUnInit() { }
	virtual void OnDetonate(CoordStruct* location) { }
	virtual void OnPut(CoordStruct pCoord, short faceDirValue8) { }
	virtual void OnRemove();
	virtual void OnReceiveDamage(args_ReceiveDamage* args);
	virtual void OnFire(AbstractClass* pTarget, int weaponIndex) { }

	virtual void OnSelect(bool& selectable) { }
	virtual void OnGuardCommand() { }
	virtual void OnStopCommand() { }
	virtual void OnDeploy() { }

	bool CanBeTargeted(WeaponTypeClass* pWeapon) const;
	bool CanBePenetrated(WarheadTypeClass* pWarhead) const;

	void BreakShield(AnimTypeClass* pBreakAnim = nullptr, WeaponTypeClass* pBreakWeapon = nullptr);
	void SetRespawn(int duration, double amount, int rate, bool resetTimer);
	void SetSelfHealing(int duration, double amount, int rate, bool resetTimer);

	void KillAnim();

	virtual void OnTemporalUpdate(TemporalClass* pTemporal);
	virtual void OnUpdate();

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
	FixedString<0x80> TechnoID;
	int HP;

		TimerStruct Timers_SelfHealing;
		TimerStruct Timers_SelfHealing_Warhead;
		TimerStruct Timers_Respawn;
		TimerStruct Timers_Respawn_Warhead;

	AnimClass* IdleAnim;
	bool Cloak;
	bool Online;
	bool Temporal;
	bool Available;
	bool Attached;
	bool AreAnimsHidden;

	double SelfHealing_Warhead;
	int SelfHealing_Rate_Warhead;
	double Respawn_Warhead;
	int Respawn_Rate_Warhead;

	int LastBreakFrame;
	double LastTechnoHealthRatio;

	ShieldTypeClass* Type;

};
