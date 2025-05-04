#pragma once
#include <TerrainClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>
#include <Utilities/Handle.h>

#include <LightSourceClass.h>
#include <CellClass.h>
#include <AnimClass.h>

class TerrainExtData final : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(TerrainExtData, "TerrainExtData")

public:
	static COMPILETIMEEVAL size_t Canary = 0xE1E2E3E4;
	using base_type = TerrainClass;
	//static COMPILETIMEEVAL size_t ExtOffset = 0xD0;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	Handle<LightSourceClass*, UninitLightSource> LighSource { nullptr };
	Handle<AnimClass*, UninitAnim> AttachedAnim { nullptr };
	Handle<AnimClass*, UninitAnim> AttachedFireAnim { nullptr };
	std::vector<CellStruct> Adjencentcells{};

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	static bool CanMoveHere(TechnoClass* pThis, TerrainClass* pTerrain);

	void InitializeLightSource();
	void InitializeAnim();

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(TerrainExtData) -
			(4u //AttachedToObject
			- 4u //inheritance
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
class NOVTABLE FakeTerrainClass : public TerrainClass
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

	void _AnimPointerExpired(AnimClass* pAnim);
};
static_assert(sizeof(FakeTerrainClass) == sizeof(TerrainClass), "Invalid Size !");