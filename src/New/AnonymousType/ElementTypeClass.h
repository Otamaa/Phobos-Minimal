#pragma once

#include <Utilities/TemplateDefB.h>

enum class ElementType : int
{
	Pyro,
	Hydro,
	Anemo,
	Electro,
	Dendro,
	Cryo,
	Geo,
	Physical,

	count
};

namespace detail
{
	template <>
	inline bool read<ElementType>(ElementType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			if (IS_SAME_STR_(parser.value(), "pyro"))
			{
				value = ElementType::Pyro;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "hydro"))
			{
				value = ElementType::Hydro;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "anemo"))
			{
				value = ElementType::Anemo;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "electro"))
			{
				value = ElementType::Electro;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "dendro"))
			{
				value = ElementType::Dendro;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "geo"))
			{
				value = ElementType::Geo;
				return true;
			}
			else if (IS_SAME_STR_(parser.value(), "physical"))
			{
				value = ElementType::Physical;
				return true;
			}
			else
			{
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected an ElementType");
			}
		}

		return false;
	}
}


class ElementTypeClass
{
public:
	Nullable<ElementType> ElementalType {};
	Valueable<int> Elemental_Duration { -1 };
	Valueable<int> Elemental_Mastery { 0 };


	ElementTypeClass(const ElementTypeClass& ele) :
		ElementalType { ele.ElementalType }
		, Elemental_Duration { ele.Elemental_Duration }
		, Elemental_Mastery { ele.Elemental_Mastery }
	{ }
};

// similar to AE ithink 
class ElementManager
{
public:
	class Elemet
	{
	public:
		TimerStruct Duration;
		std::unique_ptr<ElementTypeClass> Type {};

		Elemet(int dur, const ElementTypeClass& nElement) :
			Duration {}, Type {}
		{ 
			Duration.Start(dur);
			Type = std::move(std::make_unique<ElementTypeClass>(nElement));
		}
	};

	// why map ? 
	// these may be used for lemental raction base on GenshinImpact 
	std::map<WarheadTypeClass*  , Elemet>  ActiveElements {};

	void Apply(TechnoClass* pVictim , const ElementTypeClass& nElement)
	{ }
};