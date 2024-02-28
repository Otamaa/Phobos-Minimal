#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>
#include <GeneralStructures.h>
#include <SpecificStructures.h>
#include <CoordStruct.h>
#include <RectangleStruct.h>
#include <Point2D.h>

#include <New/Type/ShieldTypeClass.h>

enum class SelfHealingStatus : char {
	Online = 1, Offline = 2
};

class AbstractClass;
class ObjectClass;
class TechnoClass;
class AnimTypeClass;
class WarheadTypeClass;
class WeaponTypeClass;
class TechnoTypeClass;
class TemporalClass;
class ShieldClass final
{
public:
	ShieldClass();
	ShieldClass(TechnoClass* pTechno, bool isAttached);
	ShieldClass(TechnoClass* pTechno) : ShieldClass(pTechno, false) {};
	~ShieldClass() noexcept = default;

	//void OnInit() { }
	//void OnUnInit() { }
	//void OnDetonate(CoordStruct* location) { }
	//void OnPut(CoordStruct pCoord, short faceDirValue8) { }
	void OnRemove();
	void OnReceiveDamage(args_ReceiveDamage* args);
	//void OnFire(AbstractClass* pTarget, int weaponIndex) { }

	//void OnSelect(bool& selectable) { }
	//void OnGuardCommand() { }
	//void OnStopCommand() { }
	//void OnDeploy() { }

	bool CanBeTargeted(WeaponTypeClass* pWeapon) const;
	bool CanBePenetrated(WarheadTypeClass* pWarhead) const;

	void OnTemporalUpdate(TemporalClass* pTemporal);
	void OnUpdate();

	void BreakShield(AnimTypeClass* pBreakAnim = nullptr, WeaponTypeClass* pBreakWeapon = nullptr);
	void SetRespawn(int duration, double amount, int rate, bool resetTimer);
	void SetSelfHealing(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer);

	void KillAnim();

	void DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound);

	double GetHealthRatio() const
	{
		return static_cast<double>(this->HP) / static_cast<double>(this->Type->Strength);
	}

	void SetHP(int amount)
	{
		this->HP = amount > this->Type->Strength ? this->Type->Strength : amount;
	}

	int GetHP() const
	{
		return this->HP;
	}

	bool IsActive() const
	{
		return
			this->Available &&
			this->HP > 0 &&
			this->Online;
	}

	bool IsAvailable() const
	{
		return this->Available;
	}

	bool IsBrokenAndNonRespawning() const
	{
		return this->HP <= 0 && !this->Type->Respawn;
	}

	ShieldTypeClass* GetType() const
	{
		return this->Type;
	}

	Armor GetArmor() const
	{
		return this->Type->Armor;
	}

	int GetFramesSinceLastBroken() const
	{
		return Unsorted::CurrentFrame - this->LastBreakFrame;
	}

	void SetAnimationVisibility(bool visible)
	{
		if (!this->AreAnimsHidden && !visible)
			this->KillAnim();

		this->AreAnimsHidden = !visible;
	}

	static void SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo);
	static bool TEventIsShieldBroken(ObjectClass* pThis);

	bool IsGreenSP()
	{
		return (RulesExtData::Instance()->Shield_ConditionYellow * Type->Strength.Get()) < HP;
	}

	bool IsYellowSP()
	{
		return (RulesExtData::Instance()->Shield_ConditionRed * Type->Strength.Get()) < HP && HP <= (RulesExtData::Instance()->Shield_ConditionYellow * Type->Strength.Get());
	}

	bool IsRedSP()
	{
		return HP <= (RulesExtData::Instance()->Shield_ConditionRed * Type->Strength.Get());
	}

	void InvalidatePointer(AbstractClass* ptr, bool bDetach);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);

	void UpdateType();

	void SelfHealing();

	int GetPercentageAmount(double iStatus) const
	{
		if (iStatus == 0)
			return 0;

		if (iStatus >= -1.0 && iStatus <= 1.0)
			return (int)round(this->Type->Strength * iStatus);

		return (int)trunc(iStatus);
	}

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
	SelfHealingStatus SelfHealEnabledByCheck();

	void DrawShieldBar_Building(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	void DrawShieldBar_Other(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	int DrawShieldBar_Pip(const bool isBuilding);

	int DrawShieldBar_PipAmount(int iLength) const
	{
		return this->IsActive()
			? std::clamp((int)std::round(this->GetHealthRatio() * iLength), 1, iLength)
			: 0;
	}

private:

	/// Properties ///
	TechnoClass* Techno;
	TechnoTypeClass* CurTechnoType;
	int HP;

	CDTimerClass Timers_SelfHealing_CombatRestart;
	CDTimerClass Timers_SelfHealing;
	CDTimerClass Timers_SelfHealing_Warhead;
	CDTimerClass Timers_Respawn;
	CDTimerClass Timers_Respawn_Warhead;

	Handle<AnimClass* , UninitAnim> IdleAnim;
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
