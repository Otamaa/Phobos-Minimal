#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>

#include <AnimClass.h>

class AbstractClass;
class TechnoClass;
class HouseClass;
struct AEAttachParams;
class AEAttachInfoTypeClass;
class PhobosAttachEffectClass
{
public:

	void Initialize(PhobosAttachEffectTypeClass* pType, TechnoClass* pTechno, HouseClass* pInvokerHouse,
	TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay);
	void AI();
	void AI_Temporal();
	void KillAnim();
	void SetAnimationTunnelState(bool visible);

	void CreateAnim();
	void UpdateCumulativeAnim();
	void TransferCumulativeAnim(PhobosAttachEffectClass* pSource);
	bool CanShowAnim() const;

	COMPILETIMEEVAL FORCEDINLINE PhobosAttachEffectTypeClass* GetType() const {
		return this->Type;
	}

	void RefreshDuration(int durationOverride = 0);

	bool ResetIfRecreatable();

	COMPILETIMEEVAL FORCEDINLINE bool IsSelfOwned() const {
		return this->Source == this->Techno;
	}

	COMPILETIMEEVAL FORCEDINLINE bool HasExpired() const {
		return this->IsSelfOwned() && this->Delay >= 0 ? false : !this->Duration;
	}

	bool ShouldBeDiscardedNow() const;

	COMPILETIMEEVAL FORCEDINLINE bool IsActive() const {
		if (this->IsSelfOwned())
			return this->InitialDelay <= 0 && this->CurrentDelay == 0 && this->HasInitialized && this->IsOnline && !this->NeedsDurationRefresh;
		else
			return this->Duration && this->IsOnline;
	}

	COMPILETIMEEVAL FORCEDINLINE bool IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const {
		return pInvoker == this->Invoker && pSource == this->Source;
	}

	COMPILETIMEEVAL FORCEDINLINE int GetRemainingDuration() const { return this->Duration; }

	void InvalidatePointer(AbstractClass* ptr, bool removed);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static int Attach(TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, AEAttachInfoTypeClass* attachEffectInfo);
	static int Detach(TechnoClass* pTarget, AEAttachInfoTypeClass* attachEffectInfo);
	static int DetachByGroups(TechnoClass* pTarget, AEAttachInfoTypeClass* attachEffectInfo);

	static void TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget);

	~PhobosAttachEffectClass() {
		Animation.SetDestroyCondition(!Phobos::Otamaa::ExeTerminated);
	}

	void OnlineCheck();
	void CloakCheck();
	void AnimCheck();

	static PhobosAttachEffectClass* CreateAndAttach(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, HelperedVector<std::unique_ptr<PhobosAttachEffectClass>>& targetAEs, HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, AEAttachParams const& attachInfo);
	static int DetachTypes(TechnoClass* pTarget, AEAttachInfoTypeClass* attachEffectInfo, std::vector<PhobosAttachEffectTypeClass*> const& types);
	static int RemoveAllOfType(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount, int maxCount);

	template <typename T>
	bool Serialize(T& Stm);
public:

	int Duration { 0 };
	int DurationOverride { 0 };
	int Delay { 0 };
	int CurrentDelay { 0 };
	int InitialDelay { 0 };
	int RecreationDelay { -1 };
	PhobosAttachEffectTypeClass* Type { nullptr };
	TechnoClass* Techno { nullptr };
	HouseClass* InvokerHouse { nullptr };
	TechnoClass* Invoker { nullptr };
	AbstractClass* Source { nullptr };
	Handle<AnimClass*, UninitAnim> Animation { nullptr };
	bool IsAnimHidden { false };
	bool IsInTunnel { false };
	bool IsUnderTemporal { false };
	bool IsOnline { false };
	bool IsCloaked { false };
	bool HasInitialized { false };
	bool NeedsDurationRefresh { false };
	AnimTypeClass* SelectedAnim { nullptr };
	bool HasCumulativeAnim { false };
	bool ShouldBeDiscarded { false };
};

template <>
struct Savegame::ObjectFactory<PhobosAttachEffectClass>
{
	std::unique_ptr<PhobosAttachEffectClass> operator() (PhobosStreamReader& Stm) const
	{
		return std::make_unique<PhobosAttachEffectClass>();
	}
};