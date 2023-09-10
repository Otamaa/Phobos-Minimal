#pragma once
#include <BulletClass.h>

#include <Helpers/Macro.h>
//#include <Utilities/Container.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

#include <New/Entity/LaserTrailClass.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

#include "Trajectories/PhobosTrajectory.h"

class TechnoClass;
class BulletExt
{
public:
	static TechnoClass* InRangeTempFirer;

	class ExtData final : public Extension<BulletClass>
	{
	public:
		static constexpr size_t Canary = 0x2A2A2A2A;
		using base_type = BulletClass;

	public:
		int CurrentStrength { 0 };
		bool IsInterceptor { false };
		InterceptedStatus InterceptedStatus { InterceptedStatus::None };
		bool Intercepted_Detonate { true };
		std::vector<LaserTrailClass> LaserTrails {};
		bool SnappedToTarget { false };
		SuperWeaponTypeClass* NukeSW { nullptr };

		bool BrightCheckDone { false };
		HouseClass* Owner { nullptr };

		bool Bouncing { false };
		ObjectClass* LastObject { nullptr };
		int BounceAmount { 0 };
		std::vector<LineTrail*> BulletTrails {};
		OptionalStruct<DirStruct ,true> InitialBulletDir {};

		std::vector<UniversalTrail> Trails {};
		std::unique_ptr<PhobosTrajectory> Trajectory {};
		UniqueParticleSystemClassPtr AttachedSystem {};

		ExtData(BulletClass* OwnerObject) : Extension<BulletClass>(OwnerObject) { }
		virtual ~ExtData() = default;

		void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
		static bool InvalidateIgnorable(AbstractClass* ptr);

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

		void ApplyRadiationToCell(CoordStruct const& nCoord, int Spread, int RadLevel);
		void InitializeLaserTrails();

		void CreateAttachedSystem();


	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BulletExt::ExtData> {
	public:
		ExtContainer();
		~ExtContainer();

		static bool InvalidateIgnorable(AbstractClass* ptr)
		{
			switch (VTable::Get(ptr))
			{
			case BuildingClass::vtable:
			case InfantryClass::vtable:
			case UnitClass::vtable:
			case AircraftClass::vtable:
				return false;
			default:
				return true;
			}
		}

		static void InvalidatePointer(AbstractClass* ptr, bool bRemoved)
		{
			AnnounceInvalidPointer(BulletExt::InRangeTempFirer, ptr, bRemoved);
		}

		static bool LoadGlobals(PhobosStreamReader& Stm)
		{
			return Stm
				.Process(BulletExt::InRangeTempFirer)
				.Success();
		}

		static bool SaveGlobals(PhobosStreamWriter& Stm)
		{
			return Stm
				.Process(BulletExt::InRangeTempFirer)
				.Success();
		}
	};

	static ExtContainer ExtMap;

	static void InterceptBullet(BulletClass* pThis, TechnoClass* pSource, WeaponTypeClass* pWeapon);
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
};
