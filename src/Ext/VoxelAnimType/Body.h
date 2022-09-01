#pragma once
#include <VoxelAnimTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>

#include <New/Type/LaserTrailTypeClass.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#endif

class VoxelAnimTypeExt
{
public:
	static constexpr size_t Canary = 0xAAAEEEEE;
	using base_type = VoxelAnimTypeClass;
#ifdef ENABLE_NEWHOOKS
	static constexpr size_t ExtOffset = sizeof(base_type);
#endif

	class ExtData final : public Extension<VoxelAnimTypeClass>
	{
	public:

		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
		Valueable<bool> Warhead_Detonate;
#pragma region Otamaa
		NullableVector <AnimTypeClass*> SplashList;//
		Valueable<bool> SplashList_Pickrandom;
		Nullable<AnimTypeClass*> WakeAnim; //
		Valueable<bool> ExplodeOnWater;
		Valueable<bool> Damage_DealtByOwner;
		Nullable<WeaponTypeClass*> Weapon;
		Valueable<bool> ExpireDamage_ConsiderInvokerVet;
#ifdef COMPILE_PORTED_DP_FEATURES
		TrailsReader Trails;
#endif
#pragma endregion

		ExtData(VoxelAnimTypeClass* OwnerObject) : Extension<VoxelAnimTypeClass>(OwnerObject)
			, LaserTrail_Types { }
			, Warhead_Detonate { false }
			, SplashList { }
			, SplashList_Pickrandom { true }
			, WakeAnim { }
			, ExplodeOnWater { false }
			, Damage_DealtByOwner { false }
			, Weapon { }
			, ExpireDamage_ConsiderInvokerVet { false }
#ifdef COMPILE_PORTED_DP_FEATURES
			, Trails { }
#endif
		{ }

		virtual ~ExtData() = default;
		void LoadFromINIFile(CCINIClass* pINI);

		void InvalidatePointer(void* ptr, bool bRemoved) { }
		void Initialize();
		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static VoxelAnimTypeExt::ExtData* GetExtData(base_type* pThis);

	class ExtContainer final : public Container<VoxelAnimTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		void InvalidatePointer(void* ptr, bool bRemoved);
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};