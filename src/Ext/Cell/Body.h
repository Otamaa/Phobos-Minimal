#pragma once
#include <CellClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/SavegameDef.h>

class CellExtData final
{
public:

	static COMPILETIMEEVAL size_t Canary = 0x87688621;
	using base_type = CellClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	int NewPowerups { -1 };

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(CellExtData) -
			(4u //AttachedToObject
			 );
	}

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

private:
	template <typename T>
	void Serialize(T& Stm);

public:

	// Don t call it without checking Tiberium existence
	// otherwise crash
	static TiberiumClass* GetTiberium(CellClass* pCell);
	static int GetOverlayIndex(CellClass* pCell, TiberiumClass* pTiberium);
	static int GetOverlayIndex(CellClass* pCell);
};

class CellExtContainer final : public Container<CellExtData>
{
public:
	static CellExtContainer Instance;

	//CONSTEXPR_NOCOPY_CLASSB(CellExtContainer, CellExtData, "CellClass");
};