#pragma once
#include <BulletClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PooledContainer.h>
#include <Utilities/TemplateDef.h>

#include <New/Entity/LaserTrailClass.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

#include "Trajectories/PhobosTrajectory.h"
#include <Ext/Object/Body.h>

class TechnoClass;
class TechnoTypeClass;
class BulletExtData final : public ObjectExtData
{
public:
	using base_type = BulletClass;

public:
#pragma region ClassMembers

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
	OptionalStruct<DirStruct, true> InitialBulletDir {};
	std::vector<UniversalTrail> Trails {};
	std::unique_ptr<PhobosTrajectory> Trajectory {};
	ParticleSystemClass* AttachedSystem { nullptr };
	int DamageNumberOffset { INT32_MIN };
	AbstractClass* OriginalTarget { nullptr };
	int ParabombFallRate { 0 };
#pragma endregion

public:

	BulletExtData(BulletClass* pObj) : ObjectExtData(pObj) { }
	BulletExtData(BulletClass* pObj, noinit_t& nn) : ObjectExtData(pObj, nn) { }

	virtual ~BulletExtData()
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

	virtual BulletClass* This() const override { return reinterpret_cast<BulletClass*>(this->ObjectExtData::This()); }
	virtual const BulletClass* This_Const() const override { return reinterpret_cast<const BulletClass*>(this->ObjectExtData::This_Const()); }

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
	static BulletExtContainer Instance;

	static void Clear() {
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm) {
		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm) {
		return true;
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved) {
		for (auto& ext : Array) {
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual bool WriteDataToTheByteStream(BulletExtData::base_type* key, IStream* pStm) { return true;  };
	virtual bool ReadDataFromTheByteStream(BulletExtData::base_type* key, IStream* pStm) {  return true; };
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