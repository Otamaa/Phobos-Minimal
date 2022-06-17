#pragma once
#include <BulletClass.h>

#include <Helpers/Macro.h>
#include <Ext/Abstract/Body.h>
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
	using base_type = BulletClass;

	class ExtData final : public TExtension<BulletClass>
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
#ifdef COMPILE_PORTED_DP_FEATURES
		std::vector<std::unique_ptr<UniversalTrail>> Trails;
#endif
#pragma region
		PhobosTrajectory* Trajectory;

		ExtData(BulletClass* OwnerObject) : TExtension<BulletClass>(OwnerObject)
			, TypeExt { nullptr }
			, CurrentStrength { 0 }
			, IsInterceptor { false }
			, InterceptedStatus { InterceptedStatus::None }
			, Intercepted_Detonate { true }
			, LaserTrails {}
			, SnappedToTarget { false }
			, BrightCheckDone { false }
			, Owner { nullptr }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
			, Trajectory { nullptr }


		{ }

		virtual ~ExtData() = default;
		virtual size_t GetSize() const override { return sizeof(*this); }
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
		virtual void InitializeConstants() override;
		virtual void Uninitialize() override;

		void ApplyRadiationToCell(CellStruct const& Cell, int Spread, int RadLevel);
		void InitializeLaserTrails(BulletTypeExt::ExtData* pTypeExt);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	__declspec(noinline) static BulletExt::ExtData* GetExtData(base_type* pThis)
	{
		return pThis && pThis->WhatAmI() == AbstractType::Bullet ? reinterpret_cast<BulletExt::ExtData*>
			(ExtensionWrapper::GetWrapper(pThis)->ExtensionObject) : nullptr;
	}

	class ExtContainer final : public TExtensionContainer<BulletExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void InterceptBullet(BulletClass* pThis, TechnoClass* pSource, WeaponTypeClass* pWeapon);

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
