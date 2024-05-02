#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>

class AbstractClass;
class TechnoClass;
class HouseClass;
class AnimClass;
class PhobosAttachEffectClass
{
public:
	PhobosAttachEffectClass();

	PhobosAttachEffectClass(PhobosAttachEffectTypeClass* pType, TechnoClass* pTechno, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay);

	~PhobosAttachEffectClass() = default;

	void AI();
	void AI_Temporal();
	void KillAnim();
	void SetAnimationVisibility(bool visible);
	constexpr FORCEINLINE PhobosAttachEffectTypeClass* GetType() const {
		return this->Type;
	}

	void RefreshDuration(int durationOverride = 0);
	bool ResetIfRecreatable();
	constexpr FORCEINLINE bool IsSelfOwned() const {
		return this->Source == this->Techno;
	}
	constexpr FORCEINLINE bool HasExpired() const {
		return this->IsSelfOwned() && this->Delay >= 0 ? false : !this->Duration;
	}
	bool AllowedToBeActive() const;
	constexpr FORCEINLINE bool IsActive() const {
		if (this->IsSelfOwned())
			return this->InitialDelay <= 0 && this->CurrentDelay == 0 && this->HasInitialized && this->IsOnline && !this->NeedsDurationRefresh;
		else
			return this->Duration && this->IsOnline;
	}

	constexpr FORCEINLINE bool IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const {
		return pInvoker == this->Invoker && pSource == this->Source;
	}

	void ExpireWeapon() const;

	void InvalidatePointer(AbstractClass* ptr, bool removed);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static bool Attach(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, int durationOverride = 0, int delay = 0, int initialDelay = 0, int recreationDelay = -1);

	static int Attach(std::vector<PhobosAttachEffectTypeClass*> const& types, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, std::vector<int>& durationOverrides, std::vector<int> const* delays, std::vector<int> const* initialDelays, std::vector<int> const* recreationDelays);

	static int Detach(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount = -1, int maxCount = -1);
	static int Detach(std::vector<PhobosAttachEffectTypeClass*> const& types, TechnoClass* pTarget, std::vector<int> const& minCounts, std::vector<int> const& maxCounts);
	static int DetachByGroups(std::vector<std::string> const& groups, TechnoClass* pTarget, std::vector<int> const& minCounts, std::vector<int> const& maxCounts);
	static void TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget);

private:
	void OnlineCheck();
	void CloakCheck();
	void CreateAnim();

	static PhobosAttachEffectClass* CreateAndAttach(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, std::vector<std::unique_ptr<PhobosAttachEffectClass>>& targetAEs,
		HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride = 0, int delay = 0, int initialDelay = 0, int recreationDelay = -1);

	static int RemoveAllOfType(PhobosAttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount, int maxCount);

	template <typename T>
	bool Serialize(T& Stm);

	int Duration;
	int DurationOverride;
	int Delay;
	int CurrentDelay;
	int InitialDelay;
	int RecreationDelay;
	PhobosAttachEffectTypeClass* Type;
	TechnoClass* Techno;
	HouseClass* InvokerHouse;
	TechnoClass* Invoker;
	AbstractClass* Source;
	Handle<AnimClass*, UninitAnim> Animation;
	bool IsAnimHidden;
	bool IsUnderTemporal;
	bool IsOnline;
	bool IsCloaked;
	bool HasInitialized;
	bool NeedsDurationRefresh;

public:
	bool IsFirstCumulativeInstance;
};
