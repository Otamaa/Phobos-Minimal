#pragma once

#include <vector>

#include <GeneralDefinitions.h>
#include <Utilities/TemplateDef.h>

class PhobosAttachEffectClass;
class PhobosAttachEffectTypeClass;
class BulletClass;
class TechnoClass;
class WeaponTypeClass;

enum class PrismRelayPhase : unsigned char
{
	Idle = 0,
	Supporting,
	FiringMaster
};

struct PrismRelaySupportEdge
{
	TechnoClass* From { nullptr };
	TechnoClass* To { nullptr };
	int Layer { 0 };

	void OPTIONALINLINE InvalidatePointer(AbstractClass* ptr, bool bRemove)
	{
		AnnounceInvalidPointer(From, ptr, bRemove);
		AnnounceInvalidPointer(To, ptr, bRemove);
	}

public :

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<PrismRelaySupportEdge*>(this)->Serialize(Stm);
	}

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->From)
			.Process(this->To)
			.Process(this->Layer)
			;
	}
};

struct TechnoPrismRelaySession
{
	PrismRelayPhase Phase { PrismRelayPhase::Idle };
	AbstractClass* EnemyTarget { nullptr };
	WeaponTypeClass* MasterWeapon { nullptr };
	int WeaponIndex { -1 };
	int SupportCount { 0 };
	int PendingBullets { 0 };
	CDTimerClass Timeout {};
	CDTimerClass SupportFireTimer {};
	int ActiveSupportLayer { 0 };
	int PendingNextLayer { 0 };
	std::vector<TechnoClass*> NetworkNodes {};
	std::vector<PrismRelaySupportEdge> SupportEdges {};
	PhobosAttachEffectTypeClass* RelayType { nullptr };

public:

	void OPTIONALINLINE InvalidatePointer(AbstractClass* ptr, bool bRemove)
	{
		AnnounceInvalidPointer(NetworkNodes, ptr, bRemove);
		AnnounceInvalidPointer(EnemyTarget, ptr, bRemove);

		for (auto& supp : SupportEdges) {
			supp.InvalidatePointer(ptr, bRemove);
		}
	}
	void Reset()
	{
		this->Phase = PrismRelayPhase::Idle;
		this->EnemyTarget = nullptr;
		this->MasterWeapon = nullptr;
		this->WeaponIndex = -1;
		this->SupportCount = 0;
		this->PendingBullets = 0;
		this->Timeout.Stop();
		this->SupportFireTimer.Stop();
		this->ActiveSupportLayer = 0;
		this->PendingNextLayer = 0;
		this->NetworkNodes.clear();
		this->SupportEdges.clear();
		this->RelayType = nullptr;
	}

public:

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		auto _result = this->Serialize(Stm); {
		if (this->Phase != PrismRelayPhase::Idle)
			this->Reset();
		}
		return _result;
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<TechnoPrismRelaySession*>(this)->Serialize(Stm);
	}

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->Phase)
			.Process(this->EnemyTarget)
			.Process(this->MasterWeapon)
			.Process(this->WeaponIndex)
			.Process(this->SupportCount)
			.Process(this->PendingBullets)
			.Process(this->Timeout)
			.Process(this->SupportFireTimer)
			.Process(this->ActiveSupportLayer)
			.Process(this->PendingNextLayer)
			.Process(this->NetworkNodes)
			.Process(this->SupportEdges)
			.Process(this->RelayType)
			;
	}
};

class PrismRelay
{
public:
	static PhobosAttachEffectTypeClass* GetRelayType(TechnoClass* pTechno);
	static PhobosAttachEffectTypeClass* GetRelayTypeForNetwork(TechnoClass* pTechno, int networkId);
	static int GetRelayLockoutFrames(PhobosAttachEffectTypeClass* pType);
	static std::vector<TechnoClass*> BuildDamageProvidersFromEdges(const std::vector<PrismRelaySupportEdge>& edges);

	static bool CanProvide(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pType);
	static bool CanReceive(TechnoClass* pTechno, PhobosAttachEffectTypeClass* pType);
	static bool IsOnCooldown(TechnoClass* pTechno);
	static bool IsWeaponRelayAllowed(PhobosAttachEffectTypeClass* pType, WeaponTypeClass* pWeapon);
	static int ResolveMasterFireWeaponIndex(TechnoClass* pMaster, PhobosAttachEffectTypeClass* pType, AbstractClass* pTarget, int initiatingWeaponIndex);

	static int ApplyDamageBonus(int damage, const std::vector<TechnoClass*>& providers, int networkId);

	static bool TryHandleFireAt(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon, int weaponIndex);
	static void NotifyBulletDestroyed(BulletClass* pBullet);
	static void UpdateSessionTimeouts(TechnoClass* pThis);
};