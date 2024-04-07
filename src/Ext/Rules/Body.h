#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>

#include <Utilities/Container.h>

class RulesExtData final : public TExtension<RulesClass>
{
public:

	virtual RulesClass* GetAttachedObject() const override
	{
		return (RulesClass*)this->AttachedToObject;
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm)
	{
		this->TExtension<RulesClass>::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->TExtension<RulesClass>::SaveToStream(Stm);
	}

private:
	static std::unique_ptr<RulesExtData> Data;
public:
	static IStream* g_pStm;

	static void Allocate(RulesClass* pThis)
	{
		Data = std::make_unique<RulesExtData>();
		Data->AttachedToObject = pThis;
	}

	static void Remove(RulesClass* pThis)
	{
		Data = nullptr;
	}

	static RulesExtData* Instance()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(RulesClass::Instance);
	}

};