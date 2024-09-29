#pragma once

#include <FileFormats/SHP.h>

class PhobosStreamReader;
class PhobosStreamWriter;
class INI_EX;

class TheaterSpecificSHP
{
public:
	constexpr TheaterSpecificSHP() noexcept = default;

	constexpr TheaterSpecificSHP(SHPStruct* pSHP)
		: value { pSHP }
	{
	}

	constexpr operator SHPStruct* ()
	{
		return this->value;
	}

	constexpr SHPStruct* GetSHP()
	{
		return *this;
	}

	bool Read(INI_EX& parser, const char* pSection, const char* pKey);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	~TheaterSpecificSHP();
private:
	SHPStruct* value { nullptr };

protected:
	TheaterSpecificSHP(const TheaterSpecificSHP& other) = delete;
	TheaterSpecificSHP& operator=(const TheaterSpecificSHP& other) = delete;
};
