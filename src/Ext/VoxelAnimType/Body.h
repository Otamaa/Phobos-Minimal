#pragma once
#include <VoxelAnimTypeClass.h>

#include <Ext/Abstract/Body.h>
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

	class ExtData final : public TExtension<VoxelAnimTypeClass>
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

		ExtData(VoxelAnimTypeClass* OwnerObject) : TExtension<VoxelAnimTypeClass>(OwnerObject)
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

		virtual ~ExtData() override = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual bool InvalidateIgnorable(void* const ptr) const override { return true; }

		virtual void Initialize() override;
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public TExtensionContainer<VoxelAnimTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};