#pragma once

#include <Utilities/TemplateDef.h>
#include "AttachEffectBehaviour.h"
#include "EffectEnums.h"

struct EffectType
{
	Valueable<bool> Enable;
	AffectWho Affectwho;
	Valueable<bool> DeactiveWhenCivilian;

	EffectType() :
		Enable { false }
		, Affectwho { AffectWho::ALL }
		,DeactiveWhenCivilian { false }
	{ }

	virtual bool TryReadType(INI_EX& reader, const char* section) R0;

	void ReadCommonType(INI_EX& reader, const char* section, const char* title)
	{
		char Buffer[0x90];
		IMPL_SNPRNINTF(Buffer, sizeof(Buffer), "%sEnabled", title);
		Enable.Read(reader, section, Buffer);

		IMPL_SNPRNINTF(Buffer, sizeof(Buffer), "%sAffectWho", title);
		if (reader.ReadString(section, Buffer))
		{
			switch (toupper(static_cast<unsigned char>(*reader.value())))
			{
			case 'A':
				Affectwho = AffectWho::ALL;
				break;
			case 'M':
				Affectwho = AffectWho::MASTER;
				break;
			case 'S':
				Affectwho = AffectWho::STAND;
				break;
			}
		}

		IMPL_SNPRNINTF(Buffer, sizeof(Buffer), "%sDeactiveWhenCivilian", title);
		DeactiveWhenCivilian.Read(reader, section, Buffer);
	}
};
