#pragma once
#include <VoxelAnimTypeClass.h>

#include <Ext/Abstract/Body.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>
#include <Helpers/Macro.h>

#include <New/Type/LaserTrailTypeClass.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

class VoxelAnimTypeExt
{
public:
	class ExtData final : public Extension<VoxelAnimTypeClass>
	{
	public:
		static constexpr size_t Canary = 0xAAAEEEEE;
		using base_type = VoxelAnimTypeClass;

	public:

		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types { };
		Valueable<bool> Warhead_Detonate { false };
#pragma region Otamaa
		NullableVector <AnimTypeClass*> SplashList { };//
		Valueable<bool> SplashList_Pickrandom { true };
		Nullable<AnimTypeClass*> WakeAnim { }; //
		Valueable<bool> ExplodeOnWater { false };
		Valueable<bool> Damage_DealtByOwner;
		Nullable<WeaponTypeClass*> Weapon { };
		Valueable<bool> ExpireDamage_ConsiderInvokerVet { false };

		TrailsReader Trails { };
#pragma endregion

		ExtData(VoxelAnimTypeClass* OwnerObject) : Extension<VoxelAnimTypeClass>(OwnerObject)
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
		CONSTEXPR_NOCOPY_CLASS(VoxelAnimTypeExt::ExtData, "VoxelAnimTypeClass");
	};

	static ExtContainer ExtMap;
};