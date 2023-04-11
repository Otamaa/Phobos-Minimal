#pragma once
#include <BulletClass.h>

#include <Helpers/Macro.h>
//#include <Utilities/Container.h>
#include <Ext/Abstract/Body.h>
#include <Utilities/TemplateDef.h>

#include <New/Entity/LaserTrailClass.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/Trails.h>
#endif

#include "Trajectories/PhobosTrajectory.h"

class TechnoClass;
class BulletExt
{
public:
	static constexpr size_t Canary = 0x2A2A2A2A;
	using base_type = BulletClass;

	class ExtData final : public Extension<BulletClass>
	{
	public:
		int CurrentStrength;
		bool IsInterceptor;
		InterceptedStatus InterceptedStatus;
		bool Intercepted_Detonate;
		std::vector<LaserTrailClass> LaserTrails;
		bool SnappedToTarget;
#pragma region Otamaa
		bool BrightCheckDone;
		HouseClass* Owner;

		bool Bouncing;
		ObjectClass* LastObject;
		int BounceAmount;
		std::vector<LineTrail*> BulletTrails;
		OptionalStruct<DirStruct ,true> InitialBulletDir;

#ifdef COMPILE_PORTED_DP_FEATURES
		std::vector<UniversalTrail> Trails;
#endif
#pragma region
		std::unique_ptr<PhobosTrajectory> Trajectory;

		ExtData(BulletClass* OwnerObject) : Extension<BulletClass>(OwnerObject)
			, CurrentStrength { 0 }
			, IsInterceptor { false }
			, InterceptedStatus { InterceptedStatus::None }
			, Intercepted_Detonate { true }
			, LaserTrails {}
			, SnappedToTarget { false }
			, BrightCheckDone { false }
			, Owner { nullptr }

			//
			, Bouncing { false }
			, LastObject { nullptr }
			, BounceAmount { 0 }
			//
			, BulletTrails { }
			, InitialBulletDir { }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
			, Trajectory {}

		{ }

		virtual ~ExtData() = default;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
		virtual bool InvalidateIgnorable(void* const ptr) const override;
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants()override ;
	
		void ApplyRadiationToCell(CoordStruct const& nCoord, int Spread, int RadLevel);
		void InitializeLaserTrails();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BulletExt> {
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			switch ((((DWORD*)ptr)[0]))
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
	};

	static void InterceptBullet(BulletClass* pThis, TechnoClass* pSource, WeaponTypeClass* pWeapon);
	static void DetonateAt(BulletClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, CoordStruct nCoord = CoordStruct::Empty , HouseClass* pBulletOwner = nullptr);
	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static Fuse FuseCheckup(BulletClass* pBullet, CoordStruct* newlocation);
	static HouseClass* GetHouse(BulletClass* const pThis);
	static bool ApplyMCAlternative(BulletClass* pThis);
	static NOINLINE void HandleBulletRemove(BulletClass* pThis, bool bDetonate, bool bRemove);
	static NOINLINE bool IsReallyAlive(BulletClass* pThis);

	static VelocityClass GenerateVelocity(BulletClass* pThis, AbstractClass* pTarget, const int nSpeed, bool bCalculateSpeedFirst = false);
	static int GetShrapAmount(BulletClass* pThis);
	static bool AllowShrapnel(BulletClass* pThis, CellClass* pCell);
	static bool ShrapnelTargetEligible(BulletClass* pThis, AbstractClass* pTarget, bool checkOwner = true);
	static void ApplyShrapnel(BulletClass* pThis);
	static DWORD ApplyAirburst(BulletClass* pThis);

	static TechnoClass* InRangeTempFirer;
};
