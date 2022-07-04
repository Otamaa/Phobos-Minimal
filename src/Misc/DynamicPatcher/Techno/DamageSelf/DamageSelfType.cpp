#ifdef COMPILE_PORTED_DP_FEATURES
#include "DamageSelfType.h"

#include <WarheadTypeClass.h>

void DamageSelfType::Read(INI_EX& parser, const char* pSection)
{
	Nullable<WarheadTypeClass*> nWHDumMy { };
	nWHDumMy.Read(parser, pSection, "DamageSelf.Warhead");

	if (!nWHDumMy.isset())
		return;

	Warhead = nWHDumMy.Get();

	Valueable<int> nIntDummy { Damage };
	nIntDummy.Read(parser, pSection, "DamageSelf.Damage");
	Damage = nIntDummy.Get();

	nIntDummy = ROF;
	nIntDummy.Read(parser, pSection, "DamageSelf.ROF");
	ROF = nIntDummy.Get();

	Valueable<bool> nBoolDummy { PlayWarheadAnim };
	nBoolDummy.Read(parser, pSection, "DamageSelf.WarheadAnim");
	PlayWarheadAnim = nIntDummy.Get();

	nBoolDummy = IgnoreArmor;
	nBoolDummy.Read(parser, pSection, "DamageSelf.IgnoreArmor");
	IgnoreArmor = nIntDummy.Get();

	nBoolDummy = Decloak;
	nBoolDummy.Read(parser, pSection, "DamageSelf.Decloak");
	Decloak = nIntDummy.Get();

	Valueable<KillMethod> nKillType { Type };
	nKillType.Read(parser, pSection, "DamageSelf.KillType");
	Type = nKillType.Get();

}
#endif