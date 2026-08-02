#include "TheaterImageClass.h"

#include <New/Type/TheaterTypeClass.h>

#include <GameStrings.h>
#include <TechnoTypeClass.h>

#include <Utilities/INIParser.h>

TheaterIamgeClass::TheaterIamgeClass() {
	this->Types.resize(TheaterTypeClass::Array.size(), nullptr);
}

bool TheaterIamgeClass::Read(INI_EX& parser, const char* pSection, std::string_view base_key, AbstractType baseType)
{
	for (size_t i = 0; i < this->Types.size(); ++i) {
		std::string _base(base_key);
					_base += TheaterTypeClass::Array[i]->Name.data();

		if (parser.ReadString(pSection, _base.c_str())) {
			std::string _value = parser.value();

			auto pFindType = TechnoTypeClass::Find(_value.c_str());

			if(pFindType && pFindType->WhatAmI() == baseType)
				this->Types[i] = pFindType;

			if (!this->Types[i] && !GameStrings::IsBlank(_value.c_str())) {
				Debug::Log("Failed to find TechnoTypeClass referenced by [%s]%s=%s\n", pSection, _base.c_str(), parser.c_str());
			}
		}
	}
}

bool TheaterIamgeClass::Load(PhobosStreamReader& Stm, bool RegisterForChange) {
	return Stm.Process(this->Types, RegisterForChange);
}

bool TheaterIamgeClass::Save(PhobosStreamWriter& Stm) const {
	return Stm.Process(this->Types);
}

TheaterIamgeClass::operator TechnoTypeClass* () {
	
	return this->Types[(int)ScenarioClass::Instance->Theater];
}

TechnoTypeClass* TheaterIamgeClass::Get() {
	return  this->Types[(int)ScenarioClass::Instance->Theater];
}
