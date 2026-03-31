#pragma once
#include <BulletClass.h>

#include <Helpers/Macro.h>

#include <Utilities/TemplateDef.h>

#include <New/Entity/LaserTrailClass.h>

#include "Trajectories/PhobosTrajectory.h"

#include <Ext/Object/Body.h>

class TechnoClass;
class TechnoTypeClass;
class BulletExtData final : public ObjectExtData
{
public:
	using base_type = BulletClass;
	static COMPILETIMEEVAL const char* ClassName = "BulletExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "BulletClass";
	
public:
#pragma region ClassMembers
	// ============================================================
	// 8-byte aligned: Pointers
	// ============================================================
	TechnoTypeClass* InterceptorTechnoType { nullptr };
	SuperWeaponTypeClass* NukeSW { nullptr };
	HouseClass* Owner { nullptr };
	AbstractClass* OriginalTarget { nullptr };

	// ============================================================
	// 8-byte aligned: unique_ptr
	// ============================================================
	std::unique_ptr<PhobosTrajectory> Trajectory { nullptr };

	// ============================================================
	// 8-byte aligned: Handle wrapper
	// ============================================================
	Handle<ParticleSystemClass*, MarkForDeathDeleterB<ParticleSystemClass>> AttachedSystem { nullptr };

	// ============================================================
	// 24-byte aligned: Vectors
	// ============================================================
	HelperedVector<std::unique_ptr<LaserTrailClass>> LaserTrails {};

	// ============================================================
	// 4-byte aligned: int, enum
	// ============================================================
	int CurrentStrength { 0 };
	int DamageNumberOffset { INT32_MIN };
	int ParabombFallRate { 0 };
	InterceptedStatus InterceptedStatus { InterceptedStatus::None };

	// ============================================================
	// 1-byte aligned: bool (packed together at the end)
	// ============================================================
	bool DetonateOnInterception { true };
	bool SnappedToTarget { false };
	bool BrightCheckDone { false };
	bool IsInstantDetonation { false };
	// 4 bools = 4 bytes, naturally aligned
#pragma endregion

public:
	BulletExtData(BulletClass* pObj) : ObjectExtData(pObj)
	{
		this->AbsType = BulletClass::AbsID;
	}

	BulletExtData(BulletClass* pObj, noinit_t nn) : ObjectExtData(pObj, nn) { }

	virtual ~BulletExtData() = default;
	// {
	// 	// mimicking how this thing does , since the detach seems not properly handle these
	//
	// 	if (!Phobos::Otamaa::DoingLoadGame)
	// 	{
	// 		if (auto pAttach = AttachedSystem)
	// 		{
	// 			pAttach->Owner = nullptr;
	// 			pAttach->UnInit();
	// 			pAttach->TimeToDie = true;
	// 		}
	// 	}
	// }

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->ObjectExtData::SaveToStream(Stm);
		const_cast<BulletExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const {
		this->ObjectExtData::CalculateCRC(crc);
	}

	BulletClass* This() const { return reinterpret_cast<BulletClass*>(this->AttachedToObject); }
	const BulletClass* This_Const() const { return reinterpret_cast<const BulletClass*>(this->AttachedToObject); }

public:

	void ApplyRadiationToCell(CellClass* pCell, int Spread, int RadLevel);
	void InitializeLaserTrails();

	void CreateAttachedSystem();
	void ApplyArcingFix(const CoordStruct& sourceCoords, const CoordStruct& targetCoords, VelocityClass& velocity);

	static void InterceptBullet(BulletClass* pThis, TechnoClass* pSource, BulletClass* pInterceptor);
	//static void DetonateAt(BulletClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, CoordStruct nCoord = CoordStruct::Empty, HouseClass* pBulletOwner = nullptr);

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

	static COMPILETIMEEVAL const char* ClassName = "BulletExtContainer";

public:
	static BulletExtContainer Instance;

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }

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

	void _Detach(AbstractClass* target, bool all);

	FORCEDINLINE BulletClass* _AsBullet() const {
		return (BulletClass*)this;
	}

	FORCEDINLINE BulletExtData* _GetExtData() {
		return *reinterpret_cast<BulletExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE BulletTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<BulletTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}

	FORCEDINLINE FakeWarheadTypeClass* _GetWarheadType() {
		return (FakeWarheadTypeClass*)this->WH;
	}

	FORCEDINLINE WarheadTypeExtData* _GetWarheadTypeExtData() {
		return *reinterpret_cast<WarheadTypeExtData**>(((DWORD)this->WH) + AbstractExtOffset);
	}

	FORCEDINLINE FakeWeaponType* _GetWeaponType() {
		return (FakeWeaponType*)this->WeaponType;
	}

	FORCEDINLINE WeaponTypeExtData* _GetWeaponTypeExtData() {
		return *reinterpret_cast<WeaponTypeExtData**>(((DWORD)this->WeaponType) + AbstractExtOffset);
	}
};

static_assert(sizeof(FakeBulletClass) == sizeof(BulletClass), "Invalid Size !");