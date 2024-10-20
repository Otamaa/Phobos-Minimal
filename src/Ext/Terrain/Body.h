#pragma once
#include <TerrainClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>
#include <Utilities/Handle.h>

#include <LightSourceClass.h>
#include <CellClass.h>

class TerrainExtData final
{
public:
	static constexpr size_t Canary = 0xE1E2E3E4;
	using base_type = TerrainClass;
	//static constexpr size_t ExtOffset = 0xD0;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	Handle<LightSourceClass*, UninitLightSource> LighSource { nullptr };
	Handle<AnimClass*, UninitAnim> AttachedAnim { nullptr };
	std::vector<CellStruct> Adjencentcells{};

	~TerrainExtData() noexcept
	{
		LighSource.SetDestroyCondition(!Phobos::Otamaa::ExeTerminated);
		AttachedAnim.SetDestroyCondition(!Phobos::Otamaa::ExeTerminated);
	}

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	static bool InvalidateIgnorable(AbstractClass* ptr)
	{
		switch (ptr->WhatAmI())
		{
		case AnimClass::AbsID:
		case LightSourceClass::AbsID:
			return false;
		}

		return true;
	}

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	static bool CanMoveHere(TechnoClass* pThis, TerrainClass* pTerrain);

	void InitializeLightSource();
	void InitializeAnim();

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(TerrainExtData) -
			(4u //AttachedToObject
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);

public:

	static void Unlimbo(TerrainClass* pThis, CoordStruct* pCoord);
};

class TerrainExtContainer final : public Container<TerrainExtData>
{
public:
	static TerrainExtContainer Instance;

	//CONSTEXPR_NOCOPY_CLASSB(TerrainExtContainer, TerrainExtData, "TerrainClass");
};

class TerrainTypeExtData;
class FakeTerrainClass : public TerrainClass
{
public:

	void _Detach(AbstractClass* target, bool all);

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	TerrainExtData* _GetExtData() {
		return *reinterpret_cast<TerrainExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	TerrainTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<TerrainTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeTerrainClass) == sizeof(TerrainClass), "Invalid Size !");