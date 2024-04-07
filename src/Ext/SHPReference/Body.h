#pragma once

#include <FileFormats/SHP.h>

#include <Utilities/Container.h>

class SHPRefExt : public TExtension<SHPReference>
{
public:
	virtual SHPReference* GetAttachedObject() const override
	{
		return (SHPReference*)this->AttachedToObject;
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm)
	{
		this->TExtension<SHPReference>::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		this->TExtension<SHPReference>::SaveToStream(Stm);
	}
};