#pragma once
#include <Utilities/TemplateDef.h>

enum class AffectWho : int
{
	ALL = 0, MASTER = 1, STAND = 2
};

struct CommonProperties
{
	Valueable<bool> DeactiveWhenCivilian;
	AffectWho Who;

	CommonProperties() :
		DeactiveWhenCivilian { false }
		, Who { AffectWho::ALL }
	{ }

	CommonProperties(AffectWho nWho) :
		DeactiveWhenCivilian { false }
		, Who { nWho }
	{ }

	void Read(INI_EX& parser, const char* pSection, const char* pTag)
	{
		char Buffer[0x90];
		IMPL_SNPRNINTF(Buffer, sizeof(Buffer), "%sAffectWho", pTag);
		if (parser.ReadString(pSection, Buffer))
		{
			if(IS_SAME_STR_("All", parser.value()))
				Who = AffectWho::ALL;
			else if(IS_SAME_STR_("Master", parser.value()))
				Who = AffectWho::MASTER;
			else if(IS_SAME_STR_("Stand", parser.value()))
				Who = AffectWho::STAND;
		}

		IMPL_SNPRNINTF(Buffer, sizeof(Buffer), "%sDeactiveWhenCivilian", pTag);
		DeactiveWhenCivilian.Read(parser, pSection, Buffer);
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(DeactiveWhenCivilian)
			.Process(Who)
			;

		//Stm.RegisterChange(this); // announce this type
	}
};