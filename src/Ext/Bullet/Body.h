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
#ifndef ENABLE_NEWEXT
	//static constexpr size_t ExtOffset = 0xE4;
#endif

	class ExtData final : public TExtension<BulletClass>
	{
	public:
		int CurrentStrength;
		bool IsInterceptor;
		InterceptedStatus InterceptedStatus;
		bool Intercepted_Detonate;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		bool SnappedToTarget;
#pragma region Otamaa
		bool BrightCheckDone;
		HouseClass* Owner;

		bool Bouncing;
		ObjectClass* LastObject;
		int BounceAmount;
		DynamicVectorClass<LineTrail*> BulletTrails;
		OptionalStruct<DirStruct ,true> InitialBulletDir;
#ifdef COMPILE_PORTED_DP_FEATURES
		std::vector<std::unique_ptr<UniversalTrail>> Trails;
#endif
#pragma region
		std::unique_ptr<PhobosTrajectory> Trajectory;

		ExtData(BulletClass* OwnerObject) : TExtension<BulletClass>(OwnerObject)
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


		{ }



		virtual ~ExtData() = default;

		void InvalidatePointer(void* ptr, bool bRemoved);
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants();
		void Uninitialize();

		void ApplyRadiationToCell(CellStruct const& Cell, int Spread, int RadLevel);
		void InitializeLaserTrails();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<BulletExt
#ifdef AAENABLE_NEWEXT
, true
, true
#endif
	> {
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;

		//virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		//{
		//	auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
		//	switch (abs)
		//	{
		//	case AbstractType::House:
		//		return false;
		//	default:
		//		return true;
		//	}
		//}
	};

	static void InterceptBullet(BulletClass* pThis, TechnoClass* pSource, WeaponTypeClass* pWeapon);

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static NOINLINE Fuse FuseCheckup(BulletClass* pBullet, CoordStruct* newlocation);

	static TechnoClass* InRangeTempFirer;
};
