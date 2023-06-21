#pragma once

#include <Utilities/TemplateDefB.h>

//https://genshin.gg/elements/

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

std::array<const char*, (size_t)ElementType::count> ElementTypeToStrings
{
	{
		{"pyro"}, { "hydro" }, { "anemo" }, { "electro" }, { "dendro" }, { "geo" }, { "physical" }
	}
};

namespace detail
{
	template <>
	inline bool read<ElementType>(ElementType& value, INI_EX& parser, const char* pSection, const char* pKey, bool allocate)
	{
		if (parser.ReadString(pSection, pKey))
		{
			size_t i = 0;
			for (auto const& pStr : ElementTypeToStrings) {
				if (_IS_SAME_STR(pStr, parser.valur())) {
					value = ElementType(i);
					return true;
				}
				++i;
			}

			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected an ElementType");
		}

		return false;
	}
}

class ElementTypeClass
{
public:
	Valueable<ElementType> ElementalType { ElementType::Physical };
	Valueable<int> Elemental_Duration { -1 };
	Valueable<int> Elemental_Mastery { 0 };
};

enum class ReactionResult : int
{
	Spread = 0,
};

enum class ResonanceResult : int
{

};

// similar to AE ithink 
class ElementManager
{
public:
	class Elemet
	{
	public:
		TechnoClass* Invoker;
		CDTimerClass Duration;
		ElementTypeClass* Type;
		
		static bool CanReact(ElementType A, ElementType B)
		{
			switch (A)
			{
				case ElementType::Pyro:
				{

				}
				break;
			}

			return false;
		}

		Elemet(TechnoCLass* pInvoker , int dur, const ElementTypeClass* pEle) :
			Invoker { pInvoker  } , Duration { }, Type { pEle }
		{
			Duration.Start(dur);
		}
	};

	// these may be used for lemental raction base on GenshinImpact 
	std::vector<WarheadTypeClass*  , Elemet>  ActiveElements {};

	// all of these should referenced from WHTypeExt , dummy for now
	void Apply(TechnoClass* pInvoker, WarheadTypeClass* pWarhead, int dur, const ElementTypeClass* pEle)
	{
		if (ActiveElements.empty())
		{
			ActiveElements.empalace_back(pWarhead, Element { dur , element{ pInvoker.dur , pEle} });
		else
		{
			//elemental readtion ?

			bool Reacting = false;
			for (auto pEle : ActiveElements)
			{
				if(!pEle->can)
			}

		}

		}
	}
};