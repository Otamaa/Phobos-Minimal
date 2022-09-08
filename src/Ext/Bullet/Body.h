#pragma once
#include <BulletClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Entity/LaserTrailClass.h>
#include <Ext/BulletType/Body.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/Trails.h>
#endif

class PhobosTrajectory;
class BulletExt
{
public:
	static constexpr size_t Canary = 0x2A2A2A2A;
	using base_type = BulletClass;
//#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = 0xE4;
//#endif

	class ExtData final : public Extension<BulletClass>
	{
	public:
		BulletTypeExt::ExtData* TypeExt;
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

#ifdef COMPILE_PORTED_DP_FEATURES
		std::vector<std::unique_ptr<UniversalTrail>> Trails;
#endif
#pragma region
		PhobosTrajectory* Trajectory;

		ExtData(BulletClass* OwnerObject) : Extension<BulletClass>(OwnerObject)
			, TypeExt { nullptr }
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
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
			, Trajectory { nullptr }


		{ }



		virtual ~ExtData();

		void InvalidatePointer(void* ptr, bool bRemoved);
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		void InitializeConstants();
		void Uninitialize();

		void ApplyRadiationToCell(CellStruct const& Cell, int Spread, int RadLevel);
		void InitializeLaserTrails(BulletTypeExt::ExtData* pTypeExt);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static BulletExt::ExtData* GetExtData(base_type* pThis);

	class ExtContainer final : public Container<BulletExt , true> {
	public:
		ExtContainer();
		~ExtContainer();

		bool InvalidateExtDataIgnorable(void* const ptr) const
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::House:
				return false;
			default:
				return true;
			}
		}
	};

	static void InterceptBullet(BulletClass* pThis, TechnoClass* pSource, WeaponTypeClass* pWeapon);

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
