#pragma once

#include <SidebarClass.h>

#include <Utilities/Container.h>

class SidebarExtData final : TExtension<SidebarClass>
{
public:
	virtual SidebarClass* GetAttachedObject() const override
	{
		return (SidebarClass*)this->AttachedToObject;
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm)
	{
		this->TExtension<SidebarClass>::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->TExtension<SidebarClass>::SaveToStream(Stm);
	}
private:
	static std::unique_ptr<SidebarExtData> Data;

public:
	static IStream* g_pStm;
	static std::array<SHPReference*, 4u> TabProducingProgress;

	static void Allocate(SidebarClass* pThis)
	{
		Data = std::make_unique<SidebarExtData>();
		Data->AttachedToObject = pThis;
	}

	static void Remove(SidebarClass* pThis)
	{
		Data = nullptr;
	}

	static SidebarExtData* Instance()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(SidebarClass::Instance);
	}

};