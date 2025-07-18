#pragma once
#include <BulletClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PooledContainer.h>
#include <Utilities/TemplateDef.h>

#include <New/Entity/LaserTrailClass.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

#include "Trajectories/PhobosTrajectory.h"

class TechnoClass;
class TechnoTypeClass;
class BulletExtData
{
public:
	static COMPILETIMEEVAL size_t Canary = 0x2A2A2A2A;
	using base_type = BulletClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	int CurrentStrength { 0 };
	TechnoTypeClass* InterceptorTechnoType { };
	InterceptedStatus InterceptedStatus { InterceptedStatus::None };
	bool DetonateOnInterception { true };

	HelperedVector<std::unique_ptr<LaserTrailClass>> LaserTrails {};
	bool SnappedToTarget { false };
	SuperWeaponTypeClass* NukeSW { nullptr };

	bool BrightCheckDone { false };
	HouseClass* Owner { nullptr };

	bool Bouncing { false };
	ObjectClass* LastObject { nullptr };
	int BounceAmount { 0 };
	//std::vector<LineTrail*> BulletTrails {};
	OptionalStruct<DirStruct, true> InitialBulletDir {};

	std::vector<UniversalTrail> Trails {};
	std::unique_ptr<PhobosTrajectory> Trajectory {};
	ParticleSystemClass* AttachedSystem { nullptr };
	int DamageNumberOffset { INT32_MIN };

	AbstractClass* OriginalTarget { nullptr };
	int ParabombFallRate { 0 };

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void ApplyRadiationToCell(CellClass* pCell, int Spread, int RadLevel);
	void InitializeLaserTrails();

	void CreateAttachedSystem();
	void ApplyArcingFix(const CoordStruct& sourceCoords, const CoordStruct& targetCoords, VelocityClass& velocity);


	~BulletExtData()
	{
		// mimicking how this thing does , since the detach seems not properly handle these

		if (!Phobos::Otamaa::DoingLoadGame)
		{
			if (auto pAttach = AttachedSystem)
			{
				pAttach->Owner = nullptr;
				pAttach->UnInit();
				pAttach->TimeToDie = true;
			}
		}
	}

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of(){
		return sizeof(BulletExtData) -
		( 4u //AttachedToObject
		 	+ 4u //DamageNumberOffset
					- 4u //inheritance
			 );
	}

	static void InterceptBullet(BulletClass* pThis, TechnoClass* pSource, BulletClass* pInterceptor);
	static void DetonateAt(BulletClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, CoordStruct nCoord = CoordStruct::Empty, HouseClass* pBulletOwner = nullptr);

	static Fuse FuseCheckup(BulletClass* pBullet, CoordStruct* newlocation);
	static HouseClass* GetHouse(BulletClass* const pThis);
	static bool ApplyMCAlternative(BulletClass* pThis);
	static NOINLINE bool HandleBulletRemove(BulletClass* pThis, bool bDetonate, bool bRemove);
	static NOINLINE bool IsReallyAlive(BulletClass* pThis);

	static VelocityClass GenerateVelocity(BulletClass* pThis, AbstractClass* pTarget, const int nSpeed, bool bCalculateSpeedFirst = false);
	static int GetShrapAmount(BulletClass* pThis);
	static bool AllowShrapnel(BulletClass* pThis, CellClass* pCell);
	static bool ShrapnelTargetEligible(BulletClass* pThis, AbstractClass* pTarget, bool checkOwner = true);
	static void ApplyShrapnel(BulletClass* pThis);
	static void ApplyAirburst(BulletClass* pThis);
private:
	template <typename T>
	void Serialize(T& Stm);

public:

	static void SimulatedFiringUnlimbo(BulletClass* pBullet, HouseClass* pHouse, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords, bool randomVelocity);
	static void SimulatedFiringEffects(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach, bool firingEffect, bool visualEffect);
	static void SimulatedFiringAnim(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach);
	static void SimulatedFiringReport(BulletClass* pBullet);
	static void SimulatedFiringLaser(BulletClass* pBullet, HouseClass* pHouse);
	static void SimulatedFiringElectricBolt(BulletClass* pBullet);
	static void SimulatedFiringRadBeam(BulletClass* pBullet, HouseClass* pHouse);
	static void SimulatedFiringParticleSystem(BulletClass* pBullet, HouseClass* pHouse);

};

class BulletExtContainer final : public Container<BulletExtData>
{
public:
	static BulletExtContainer Instance;
	static ObjectPool<BulletExtData> pools;

	BulletExtData* AllocateUnchecked(BulletClass* key)
	{
		BulletExtData* val = pools.allocate();

		if (val) {
			val->AttachedToObject = key;
		}
		else
		{
			Debug::FatalErrorAndExit("The amount of [BulletExtData] is exceeded the ObjectPool size %d !", pools.getPoolSize());
		}

		return val;
	}

	void Remove(BulletClass* key)
	{
		if (BulletExtData* Item = TryFind(key))
		{
			RemoveExtOf(key, Item);
		}
	}

	void RemoveExtOf(BulletClass* key, BulletExtData* Item)
	{
		pools.deallocate(Item);
		this->ClearExtAttribute(key);
	}
	//CONSTEXPR_NOCOPY_CLASSB(BulletExtContainer, BulletExtData, "BulletClass");
};

class FakeWarheadTypeClass;
class WarheadTypeExtData;
class BulletTypeExtData;
class WeaponTypeExtData;
class FakeWeaponType;
class NOVTABLE FakeBulletClass : public BulletClass
{
public:

	void _AnimPointerExpired(AnimClass* pTarget)
	{
		this->ObjectClass::AnimPointerExpired(pTarget);
	}

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	void _Detach(AbstractClass* target, bool all);

	FORCEDINLINE BulletClass* _AsBullet() const {
		return (BulletClass*)this;
	}

	FORCEDINLINE BulletExtData* _GetExtData() {
		return *reinterpret_cast<BulletExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE BulletTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<BulletTypeExtData**>(((DWORD)this->Type) + 0x2C4);
	}

	FORCEDINLINE FakeWarheadTypeClass* _GetWarheadType() {
		return (FakeWarheadTypeClass*)this->WH;
	}

	FORCEDINLINE WarheadTypeExtData* _GetWarheadTypeExtData() {
		return *reinterpret_cast<WarheadTypeExtData**>(((DWORD)this->WH) + 0x1CC);
	}

	FORCEDINLINE FakeWeaponType* _GetWeaponType() {
		return (FakeWeaponType*)this->WeaponType;
	}

	FORCEDINLINE WeaponTypeExtData* _GetWeaponTypeExtData() {
		return *reinterpret_cast<WeaponTypeExtData**>(((DWORD)this->WeaponType) + AbstractExtOffset);
	}
};

static_assert(sizeof(FakeBulletClass) == sizeof(BulletClass), "Invalid Size !");