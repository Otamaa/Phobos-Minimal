#include "Body.h"

#include <Utilities/Macro.h>
#include <string>

///Phobos/pull/706

namespace ActionsString
{
	std::string ActionsString;
	std::deque<std::string> SubStrings;
}

DEFINE_HOOK(0x727544, TriggerClass_LoadFromINI_Actions, 0x5)
{
	GET(const char*, pString, EDX);
	ActionsString::ActionsString = pString;
	std::stringstream sin { ActionsString::ActionsString };
	std::deque<std::string>& substrs = ActionsString::SubStrings;
	substrs.clear();
	std::string tmp;
	while (std::getline(sin, tmp, ','))
	{
		substrs.emplace_back(tmp);
	}
	if (!ActionsString::SubStrings.empty())
		ActionsString::SubStrings.pop_front();
	return 0;
}

DEFINE_HOOK(0x6DD5B0, TActionClass_LoadFromINI_Parm, 0x5)
{
	GET(TActionClass*, pThis, ECX);

	if (const auto pExt = TActionExt::ExtMap.Find(pThis))
	{
		std::deque<std::string>& substrs = ActionsString::SubStrings;

		if (substrs.empty())
			return 0;

		substrs.pop_front();

		if (substrs.empty())
			return 0;

		pExt->Value1 = substrs.front();
		substrs.pop_front();

		if (substrs.empty())
			return 0;

		pExt->Value2 = substrs.front();
		substrs.pop_front();

		if (substrs.empty())
			return 0;

		pExt->Parm3 = substrs.front();
		substrs.pop_front();

		if (substrs.empty())
			return 0;

		pExt->Parm4 = substrs.front();
		substrs.pop_front();

		if (substrs.empty())
			return 0;

		pExt->Parm5 = substrs.front();
		substrs.pop_front();

		if (substrs.empty())
			return 0;

		pExt->Parm6 = substrs.front();
		substrs.pop_front();

		if (substrs.empty())
			return 0;

		substrs.pop_front();
	}
	return 0;
}