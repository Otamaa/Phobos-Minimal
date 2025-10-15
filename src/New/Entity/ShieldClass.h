#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>
#include <GeneralStructures.h>
#include <SpecificStructures.h>
#include <CoordStruct.h>
#include <RectangleStruct.h>
#include <Point2D.h>

#include <New/Type/ShieldTypeClass.h>

#include <AnimClass.h>
#include <Utilities/MemoryPoolUniquePointer.h>

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
class ShieldClass
{
public:
	ShieldClass();
	ShieldClass(TechnoClass* pTechno, bool isAttached);
	ShieldClass(TechnoClass* pTechno) : ShieldClass(pTechno, false) {};

	~ShieldClass()
	{
		Array.remove(this);
	}


	void CopyDataTo(ShieldClass& other) const
	{
		// Copy plain data fields
		other.HP = this->HP;
		other.Cloak = this->Cloak;
		other.Online = this->Online;
		other.Temporal = this->Temporal;
		other.Available = this->Available;
		other.Attached = this->Attached;
		other.AreAnimsHidden = this->AreAnimsHidden;
		other.SelfHealing_Warhead = this->SelfHealing_Warhead;
		other.SelfHealing_Rate_Warhead = this->SelfHealing_Rate_Warhead;
		other.SelfHealing_RestartInCombat_Warhead = this->SelfHealing_RestartInCombat_Warhead;
		other.SelfHealing_RestartInCombatDelay_Warhead = this->SelfHealing_RestartInCombatDelay_Warhead;
		other.Respawn_Warhead = this->Respawn_Warhead;
		other.Respawn_Rate_Warhead = this->Respawn_Rate_Warhead;
		other.Respawn_RestartInCombat_Warhead = this->Respawn_RestartInCombat_Warhead;
		other.Respawn_RestartInCombatDelay_Warhead = this->Respawn_RestartInCombatDelay_Warhead;
		other.LastBreakFrame = this->LastBreakFrame;
		other.LastTechnoHealthRatio = this->LastTechnoHealthRatio;

		// Copy timers
		other.Timers = this->Timers;

		// Copy Type pointer (shared, not owned)
		other.Type = this->Type;
		other.CurTechnoType = this->CurTechnoType;

		// Copy vector
		other.Respawn_Anim_Warhead = this->Respawn_Anim_Warhead;

		// Copy weapon pointer
		other.Respawn_Weapon_Warhead = this->Respawn_Weapon_Warhead;
	}

	//void OnInit() { }
	//void OnUnInit() { }
	//void OnDetonate(CoordStruct* location) { }
	//void OnPut(CoordStruct pCoord, short faceDirValue8) { }
	void OnRemove();
	int OnReceiveDamage(args_ReceiveDamage* args);
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
	void SetRespawn(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer, std::vector<AnimTypeClass*>* anim , WeaponTypeClass* weapon = nullptr);
	void SetSelfHealing(int duration, double amount, int rate, bool restartInCombat, int restartInCombatDelay, bool resetTimer);

	void KillAnim();
	void SetRespawnRestartInCombat();

	void DrawShieldBar(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	void DrawShieldBar_Building(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	void DrawShieldBar_Other(int iLength, Point2D* pLocation, RectangleStruct* pBound);
	int DrawShieldBar_Pip(const bool isBuilding);

	COMPILETIMEEVAL FORCEDINLINE double GetHealthRatio() const
	{
		return static_cast<double>(this->HP) / static_cast<double>(this->Type->Strength);
	}

	COMPILETIMEEVAL FORCEDINLINE void SetHP(int amount)
	{
		this->HP = amount > this->Type->Strength ? this->Type->Strength : amount;
	}

	COMPILETIMEEVAL FORCEDINLINE int GetHP() const
	{
		return this->HP;
	}

	COMPILETIMEEVAL FORCEDINLINE bool IsActive() const
	{
		return
			this->Available &&
			this->HP > 0 &&
			this->Online;
	}

	COMPILETIMEEVAL FORCEDINLINE bool IsAvailable() const
	{
		return this->Available;
	}

	COMPILETIMEEVAL FORCEDINLINE bool IsBrokenAndNonRespawning() const
	{
		return this->HP <= 0 && !this->Type->Respawn;
	}

	COMPILETIMEEVAL FORCEDINLINE bool HasTint() const {
		return this->Type->Tint_Color.isset() || this->Type->Tint_Intensity != 0.0;
	}

	COMPILETIMEEVAL FORCEDINLINE ShieldTypeClass* GetType() const
	{
		return this->Type;
	}

	COMPILETIMEEVAL FORCEDINLINE Armor GetArmor(Armor inherit) const
	{
		const auto pShieldType = this->Type;

		if (pShieldType->InheritArmorFromTechno)
		{
			const auto pTechnoType = this->Techno->GetTechnoType();

			if (pShieldType->InheritArmor_Allowed.empty() || pShieldType->InheritArmor_Allowed.Contains(pTechnoType)
				&& (pShieldType->InheritArmor_Disallowed.empty() || !pShieldType->InheritArmor_Disallowed.Contains(pTechnoType)))
				return inherit;
		}

		return pShieldType->Armor.Get();
	}

	Armor GetOrInheritArmor() const;

	COMPILETIMEEVAL FORCEDINLINE int GetFramesSinceLastBroken() const
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

	COMPILETIMEEVAL FORCEDINLINE bool IsGreenSP()
	{
		return (RulesExtData::Instance()->Shield_ConditionYellow * Type->Strength.Get()) < HP;
	}

	COMPILETIMEEVAL FORCEDINLINE bool IsYellowSP()
	{
		return (RulesExtData::Instance()->Shield_ConditionRed * Type->Strength.Get()) < HP && HP <= (RulesExtData::Instance()->Shield_ConditionYellow * Type->Strength.Get());
	}

	COMPILETIMEEVAL FORCEDINLINE bool IsRedSP()
	{
		return HP <= (RulesExtData::Instance()->Shield_ConditionRed * Type->Strength.Get());
	}

	void UpdateTint(bool forceUpdate = false);

	void InvalidateAnimPointer(AnimClass *ptr);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static HelperedVector<ShieldClass*> Array;
	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return Stm
			.Process(Array);
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return Stm
			.Process(Array);
	}

private:
	template <typename T>
	bool Serialize(T& Stm);

	void UpdateType();

	void SelfHealing();

	COMPILETIMEEVAL FORCEDINLINE int GetPercentageAmount(double iStatus) const
	{
		if (iStatus == 0)
			return 0;

		if (iStatus >= -1.0 && iStatus <= 1.0)
			return (int)std::round(this->Type->Strength * iStatus);

		return (int)std::trunc(iStatus);
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

	COMPILETIMEEVAL FORCEDINLINE int DrawShieldBar_PipAmount(int iLength) const
	{
		return this->IsActive()
			? std::clamp((int)std::round(this->GetHealthRatio() * iLength), 1, iLength)
			: 0;
	}

public:

	/// Properties ///
	TechnoClass* Techno;
	TechnoTypeClass* CurTechnoType;
	int HP;

	struct Timers {

		bool Load(PhobosStreamReader& Stm, bool RegisterForChange) {

			return Stm
				.Process(this->SelfHealing_CombatRestart)
				.Process(this->SelfHealing)
				.Process(this->SelfHealing_Warhead)
				.Process(this->Respawn)
				.Process(this->Respawn_Warhead)
				.Success();
		}

		bool Save(PhobosStreamWriter& Stm) const {

			return Stm
				.Process(this->SelfHealing_CombatRestart)
				.Process(this->SelfHealing)
				.Process(this->SelfHealing_Warhead)
				.Process(this->Respawn_CombatRestart)
				.Process(this->Respawn)
				.Process(this->Respawn_Warhead)
				.Success();
		}

		CDTimerClass SelfHealing_CombatRestart;
		CDTimerClass SelfHealing;
		CDTimerClass SelfHealing_Warhead;
		CDTimerClass Respawn_CombatRestart;
		CDTimerClass Respawn;
		CDTimerClass Respawn_Warhead;

	} Timers;

	MarkPtr<AnimClass> IdleAnim;
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

	bool Respawn_RestartInCombat_Warhead;
	int Respawn_RestartInCombatDelay_Warhead;
	HelperedVector<AnimTypeClass*> Respawn_Anim_Warhead;
	WeaponTypeClass* Respawn_Weapon_Warhead;

	int LastBreakFrame;
	double LastTechnoHealthRatio;

	ShieldTypeClass* Type;

};

template <>
struct Savegame::ObjectFactory<ShieldClass>
{
	std::unique_ptr<ShieldClass> operator() (PhobosStreamReader& Stm) const
	{
		return std::make_unique<ShieldClass>();
	}
};