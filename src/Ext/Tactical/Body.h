#pragma once

#include <TacticalClass.h>
#include <Utilities/Container.h>

class TacticalExt : public TExtension<TacticalClass>
{
public:
	virtual TacticalClass* GetAttachedObject() const override
	{
		return (TacticalClass*)this->AttachedToObject;
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm)
	{
		this->TExtension<TacticalClass>::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->TExtension<TacticalClass>::SaveToStream(Stm);
	}
private:
	static std::unique_ptr<TacticalExt> Data;
public:

	static void Allocate(TacticalClass* pThis)
	{
		Data = std::make_unique<TacticalExt>();
		Data->AttachedToObject = pThis;
	}

	static void Remove(TacticalClass* pThis)
	{
		Data = nullptr;
	}

	static TacticalExt* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(TacticalClass::Instance);
	}
};