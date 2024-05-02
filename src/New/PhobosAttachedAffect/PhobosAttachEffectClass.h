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
	PhobosAttachEffectTypeClass* GetType() const;
	void RefreshDuration(int durationOverride = 0);
	bool ResetIfRecreatable();
	bool IsSelfOwned() const;
	bool HasExpired() const;
	bool AllowedToBeActive() const;
	bool IsActive() const;
	bool IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const;

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
