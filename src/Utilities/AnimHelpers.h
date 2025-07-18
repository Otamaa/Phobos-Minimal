#pragma once

#include <Ext/WarheadType/Body.h>
#include <Ext/Techno/Body.h>
#include <Misc/DamageArea.h>

#include "TemplateDef.h"

namespace Helper
{
	namespace Otamaa
	{
		OPTIONALINLINE AnimTypeClass* PickSplashAnim(NullableVector<AnimTypeClass*> const& nSplash, Nullable<AnimTypeClass*> const& nWake, bool Random, bool IsMeteor)
		{
			if (nSplash.HasValue()) {
				if (nSplash.size() > 0) {
					return nSplash[Random ? ScenarioClass::Instance->Random.RandomFromMax((nSplash.size() - 1)) : IsMeteor ? nSplash.size() - 1 : 0];
				}
			}

			return !IsMeteor && nWake.isset()  ? nWake.Get() : RulesClass::Instance->Wake;
		}

		OPTIONALINLINE std::pair<bool, int> DetonateWarhead(int nDamage, WarheadTypeClass* pWarhead, bool bWarheadDetonate, CoordStruct Where, TechnoClass* pInvoker, HouseClass* pOwner, bool DamageConsiderVet)
		{
			if (pWarhead)
			{
				auto nResultDamage = static_cast<int>(TechnoExtData::GetDamageMult(pInvoker, nDamage , !DamageConsiderVet));

				if (bWarheadDetonate)
				{
					WarheadTypeExtData::DetonateAt(pWarhead, Where, pInvoker, nResultDamage, pOwner);
				}
				else
				{
					DamageArea::Apply(&Where, nResultDamage, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
					MapClass::FlashbangWarheadAt(nResultDamage, pWarhead, Where);
					return { true, nResultDamage };
				}
			}

			return { false , 0 };
		}

		OPTIONALINLINE std::pair<bool ,int> Detonate(Nullable<WeaponTypeClass*> const& pWeapon, int nDamage, WarheadTypeClass* pWarhead, bool bWarheadDetonate, CoordStruct Where, TechnoClass* pInvoker, HouseClass* pOwner, bool DamageConsiderVet)
		{
			if (!pWeapon.isset()) {
				return DetonateWarhead(nDamage, pWarhead, bWarheadDetonate, Where, pInvoker, pOwner, DamageConsiderVet);
			}

			auto nResultDamage = static_cast<int>(TechnoExtData::GetDamageMult(pInvoker, nDamage , !DamageConsiderVet));
			WeaponTypeExtData::DetonateAt(pWeapon, Where, pInvoker, nResultDamage, false);
			return { false , 0 };
		}

		OPTIONALINLINE void SpawnMultiple(const std::vector<AnimTypeClass*>& nAnims, std::vector<int>& nAmount,CoordStruct Where, TechnoClass* pInvoker, HouseClass* pOwner, bool bRandom)
		{
			if (!nAnims.empty())
			{
				auto nCreateAnim = [&](int nIndex)
				{
					if (auto const pMultipleSelected = nAnims[nIndex])
					{
						for (int k = nAmount[nIndex]; k > 0; --k)
						{
							AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pMultipleSelected, Where),
								pOwner,
								nullptr,
								pInvoker,
								false, false
							) ;
						}
					}
				};

				if (!bRandom)
				{
					for (int i = 0; i < (int)nAnims.size(); i++)
					{
						nCreateAnim(i);
					}
				}
				else
				{
					nCreateAnim(ScenarioClass::Instance->Random.RandomFromMax(nAnims.size() - 1));
				}
			}
		}

		OPTIONALINLINE std::tuple<bool ,int , int> CheckMinMax(double nMin, double nMax)
		{
			int nMinL = (int)(Math::abs(nMin) * 256.0);
			int nMaxL = (int)(Math::abs(nMax) * 256.0);

			if (!nMinL && !nMaxL)
				return {false ,0,0};

			if (nMinL > nMaxL)
				std::swap(nMinL, nMaxL);

			return { true ,nMinL,nMaxL };
		}

		OPTIONALINLINE CoordStruct GetRandomCoordsInsideLoops(double nMin, double nMax, CoordStruct nPos, int Increment)
		{
			auto const& [nMinMax, nMinL, nMaxL] = CheckMinMax(nMin, nMax);

			if (nMinMax) {
				auto nRandomCoords = MapClass::GetRandomCoordsNear(nPos,
					(Math::abs(ScenarioClass::Instance->Random.RandomRanged(nMinL, nMaxL)) *
					MaxImpl(Increment, 1)),
					ScenarioClass::Instance->Random.RandomBool());

				nRandomCoords.Z = nPos.Z + MapClass::Instance->GetCellFloorHeight(nRandomCoords);
				return nRandomCoords;
			}

			return nPos;
		}
	}
}