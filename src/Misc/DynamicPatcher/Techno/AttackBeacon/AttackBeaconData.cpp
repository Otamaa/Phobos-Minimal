#include "AttackBeaconData.h"

void AttackBeaconData::Read(INI_EX& parserRules, const char* pSection_rules)
{

	Types.Read(parserRules, pSection_rules, "AttackBeacon.Types");

	if (!Types.empty())
	{
		auto const nBaseSize = (int)Types.size();
		Num.Clear();
		Num.Reserve(nBaseSize);
		Num.Count = nBaseSize;
		auto const pNumKey = "AttackBeacon.Num";

		for (auto& nSpawnMult : Num)
			nSpawnMult = -1;

		if (parserRules.ReadString(pSection_rules, pNumKey))
		{
			int nCount = 0;
			char* context = nullptr;
			for (char* cur = strtok_s(parserRules.value(), Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				//if (Phobos::Config::MoreDetailSLDebugLog)
					//Debug::LogInfo("Parsing %d Size of [%s]%s=%s idx[%d] ", nBaseSize, pSection_rules, pNumKey, cur, nCount);

				int buffer;
				if (Parser<int>::TryParse(cur, &buffer))
					Num[nCount] = buffer;

				if (++nCount >= nBaseSize)
					break;
			}
		}

		Rate.Read(parserRules, pSection_rules, "AttackBeacon.Rate");
		Delay.Read(parserRules, pSection_rules, "AttackBeacon.Delay");
		RangeMin.Read(parserRules, pSection_rules, "AttackBeacon.RangeMin");
		RangeMax.Read(parserRules, pSection_rules, "AttackBeacon.RangeMax");
		Close.Read(parserRules, pSection_rules, "AttackBeacon.Close");
		Force.Read(parserRules, pSection_rules, "AttackBeacon.Force");
		Count.Read(parserRules, pSection_rules, "AttackBeacon.Count");
		TargetToCell.Read(parserRules, pSection_rules, "AttackBeacon.TargetToCell");
		AffectsOwner.Read(parserRules, pSection_rules, "AttackBeacon.AffectsOwner");
		AffectsAllies.Read(parserRules, pSection_rules, "AttackBeacon.AffectsAllies");
		AffectsEnemies.Read(parserRules, pSection_rules, "AttackBeacon.AffectsEnemies");

	}

}
