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
	class ExtData final : public Extension<VoxelAnimTypeClass>
	{
	public:
		static constexpr size_t Canary = 0xAAAEEEEE;
		using base_type = VoxelAnimTypeClass;

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

		virtual ~ExtData() override = default;

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
		void Initialize();

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<VoxelAnimTypeExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};