#pragma once
#include <VoxelAnimTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/LaserTrailTypeClass.h>
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

class VoxelAnimTypeExtData final
{
public:
	static constexpr size_t Canary = 0xAAAEEEEE;
	using base_type = VoxelAnimTypeClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
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

	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void Initialize();

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(VoxelAnimTypeExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);
};

class VoxelAnimTypeExtContainer final : public Container<VoxelAnimTypeExtData>
{
public:
	static VoxelAnimTypeExtContainer Instance;

	//CONSTEXPR_NOCOPY_CLASSB(VoxelAnimTypeExtContainer, VoxelAnimTypeExtData, "VoxelAnimTypeClass");
};
