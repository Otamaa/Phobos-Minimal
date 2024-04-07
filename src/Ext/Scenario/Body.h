#pragma once

#include <ScenarioClass.h>

#include <Utilities/Container.h>

class ScenarioExtData final : public TExtension<ScenarioClass>
{
public:
	virtual ScenarioClass* GetAttachedObject() const override
	{
		return (ScenarioClass*)this->AttachedToObject;
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm)
	{
		this->TExtension<ScenarioClass>::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->TExtension<ScenarioClass>::SaveToStream(Stm);
	}
private:
	static std::unique_ptr<ScenarioExtData> Data;

public:
	static IStream* g_pStm;
	static bool CellParsed;

	static void Allocate(ScenarioClass* pThis)
	{
		Data = std::make_unique<ScenarioExtData>();
		Data->AttachedToObject = pThis;
	}

	static void Remove(ScenarioClass* pThis)
	{
		Data = nullptr;
	}


	static ScenarioExtData* Instance()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(ScenarioClass::Instance);
	}
};