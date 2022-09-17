#pragma once
#include <BulletClass.h>
#include <WeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/RadTypeClass.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Others/DamageText.h>
#include <Misc/DynamicPatcher/Weapon/AttachFireData.h>
#endif

class WeaponTypeExt
{
public:
	static constexpr size_t Canary = 0x22222222;
	using base_type = WeaponTypeClass;
//#ifdef ENABLE_NEWEXT
	static constexpr size_t ExtOffset = 0x118;
//#endif

	class ExtData final : public Extension<WeaponTypeClass>
	{
	public:

		Valueable<double> DiskLaser_Radius;
		Valueable<int> DiskLaser_Circumference;
		Valueable<RadTypeClass*> RadType;
		Valueable<bool> Rad_NoOwner;
		Valueable<bool> Bolt_Disable1;
		Valueable<bool> Bolt_Disable2;
		Valueable<bool> Bolt_Disable3;
		Valueable<int> Strafing_Shots;
		Valueable<bool> Strafing_SimulateBurst;
		Valueable<AffectedTarget> CanTarget;
		Valueable<AffectedHouse> CanTargetHouses;
		ValueableVector<int> Burst_Delays;
		Valueable<AreaFireTarget> AreaFire_Target;
		Nullable<WeaponTypeClass*> FeedbackWeapon;
		Valueable<bool> Laser_IsSingleColor;
		Valueable<double> Trajectory_Speed;
		Valueable<bool> Abductor;
#pragma region Otamaa
		Valueable<int>Xhi;
		Valueable<int>Xlo;
		Valueable<int>Yhi;
		Valueable<int>Ylo;
		Valueable<bool> ShakeLocal;

		ValueableVector<AnimTypeClass*> OccupantAnims;
		Valueable<bool> OccupantAnim_UseMultiple;
		Valueable<bool>Range_IgnoreVertical;
		// brought back from TS
		Valueable<Leptons> ProjectileRange;
		Nullable<bool> Decloak_InstantFire;
		Valueable<AnimTypeClass*> Feedback_Anim;
		Valueable<CoordStruct> Feedback_Anim_Offset;
		Valueable<bool> Feedback_Anim_UseFLH;
		Valueable<bool> DestroyTechnoAfterFiring;
		Valueable<bool> RemoveTechnoAfterFiring;
		Valueable<AnimTypeClass*> OpentoppedAnim;
		#ifdef  COMPILE_PORTED_DP_FEATURES
		Valueable<float> RockerPitch;
		AttachFireData MyAttachFireDatas;
		#endif

#pragma endregion

		ExtData(WeaponTypeClass* OwnerObject) : Extension<WeaponTypeClass>(OwnerObject)
			, DiskLaser_Radius { 38.2 }
			, DiskLaser_Circumference { 240 }
			, RadType {}
			, Rad_NoOwner { true }
			, Bolt_Disable1 { false }
			, Bolt_Disable2 { false }
			, Bolt_Disable3 { false }
			, Strafing_Shots { 5 }
			, Strafing_SimulateBurst { false }
			, CanTarget { AffectedTarget::All }
			, CanTargetHouses { AffectedHouse::All }
			, Burst_Delays {}
			, AreaFire_Target { AreaFireTarget::Base }
			, FeedbackWeapon {}
			, Laser_IsSingleColor { false }
			, Trajectory_Speed { 100.0 }
			, Abductor { false }

			, Xhi { 0 }
			, Xlo { 0 }
			, Yhi { 0 }
			, Ylo { 0 }
			, ShakeLocal { false }
			, OccupantAnims {}
			, OccupantAnim_UseMultiple { false }
			, Range_IgnoreVertical { false }
			, ProjectileRange { Leptons(100000) }
			, Decloak_InstantFire { }
			, Feedback_Anim { nullptr }
			, Feedback_Anim_Offset{{ 0,0,0 } }
			, Feedback_Anim_UseFLH { true }
			, DestroyTechnoAfterFiring { false }
			, RemoveTechnoAfterFiring { false }
			, OpentoppedAnim { nullptr }
			 #ifdef COMPILE_PORTED_DP_FEATURES
			, RockerPitch { 0.0f }
			, MyAttachFireDatas { }
			 #endif

		{ }

		virtual ~ExtData() = default;

		void LoadFromINIFile(CCINIClass* pINI);
		void Initialize();
		// void InvalidatePointer(void* ptr, bool bRemoved) { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		int GetProjectileRange() const {
			return this->ProjectileRange.Get();
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final :public Container<WeaponTypeExt
//#ifdef ENABLE_NEWEXT
		, true
//#endif
	>
	{
	public:
		ExtContainer();
		~ExtContainer();
		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;

	static WeaponTypeClass* Temporal_WP;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int nOldCircumference;

	static void DetonateAt(WeaponTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner);
	static void DetonateAt(WeaponTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage);
};
