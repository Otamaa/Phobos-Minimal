#pragma once

#include <typeinfo>

#include <AbstractTypeClass.h>

#include <Utilities/TemplateDefB.h>
#include <Misc/DynamicPatcher/Helpers/Kratos.CRT.h>


class INIConfigInterface {
	virtual void Read(INI_EX& ini) = 0;
	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) = 0;
	virtual bool Save(PhobosStreamWriter& stream) const = 0;
};

class INIConfig : public INIConfigInterface
{
public:
	AbstractTypeClass* Type = nullptr;
	bool Enable = false;

	virtual void Read(INI_EX& ini) { };

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->Type)
			.Process(this->Enable)
			.Success();
	};

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange)
	{
		return this->Serialize(stream);
	}
	virtual bool Save(PhobosStreamWriter& stream) const
	{
		return const_cast<INIConfig*>(this)->Serialize(stream);
	}

#pragma endregion
};
