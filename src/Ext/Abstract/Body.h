#pragma once
#include <AbstractClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Helpers/Macro.h>

class AbstractExtData : public TExtension<AbstractClass> {
public:

	virtual AbstractClass* GetAttachedObject() const override
	{
		return static_cast<AbstractClass*>(this->AttachedToObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->TExtension<AbstractClass>::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override const
	{
		this->TExtension<AbstractClass>::SaveToStream(Stm);
	}

	static constexpr FORCEINLINE int GetSavedOffsetSize()
	{
		return  TExtension<AbstractClass>::GetSavedOffsetSize();
	}
};