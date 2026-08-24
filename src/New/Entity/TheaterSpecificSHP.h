#pragma once
#include <Base/Macros.h>

class PhobosStreamReader;
class PhobosStreamWriter;
class INI_EX;
struct SHPCaches;

class TheaterSpecificSHP
{
public:
	TheaterSpecificSHP() noexcept = default;
	TheaterSpecificSHP(SHPCaches* pSHP);
	~TheaterSpecificSHP();

	COMPILETIMEEVAL operator SHPCaches* ()
	{
		return this->value;
	}

	COMPILETIMEEVAL SHPCaches* GetSHP()
	{
		return *this;
	}

	bool Read(INI_EX& parser, const char* pSection, const char* pKey);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	SHPCaches* value;

protected:
	TheaterSpecificSHP(const TheaterSpecificSHP& other) = delete;
	TheaterSpecificSHP& operator=(const TheaterSpecificSHP& other) = delete;
};
