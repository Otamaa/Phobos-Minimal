#pragma once

#include <Base/Always.h>
#include <GeneralDefinitions.h>

#include <vector>
#include <string>

class TechnoTypeClass;
class PhobosStreamReader;
class PhobosStreamWriter;
class INI_EX;

class TheaterIamgeClass
{
private:

	std::vector<TechnoTypeClass*> Types {};

public:

	TheaterIamgeClass();
	~TheaterIamgeClass() = default;

public:

	operator TechnoTypeClass* ();
	TechnoTypeClass* Get();

	bool Read(INI_EX& parser, const char* pSection, std::string_view base_key, AbstractType baseType);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

protected:
	TheaterIamgeClass(const TheaterIamgeClass& other) = delete;
	TheaterIamgeClass& operator=(const TheaterIamgeClass& other) = delete;
};