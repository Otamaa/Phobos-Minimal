#pragma once

#include <Ext/WarheadType/Body.h>
#include <Ext/Techno/Body.h>
#include "TemplateDef.h"

namespace Helper
{
	namespace Otamaa
	{
		inline AnimTypeClass* PickSplashAnim(NullableVector<AnimTypeClass*> const& nSplash, Nullable<AnimTypeClass*> const& nWake, bool Random, bool IsMeteor)
		{
			if (nSplash.HasValue()) {
				if (nSplash.size() > 0) {
					return nSplash[Random ? ScenarioClass::Instance->Random.RandomFromMax((nSplash.size() - 1)) : 0];
				}
			}

			return !IsMeteor && nWake.isset()  ? nWake.Get() : RulesClass::Instance->Wake;
		}

		inline std::pair<bool, int> DetonateWarhead(int nDamage, WarheadTypeClass* pWarhead, bool bWarheadDetonate, const CoordStruct& Where, TechnoClass* pInvoker, HouseClass* pOwner, bool DamageConsiderVet)
		{
			if (pWarhead)
			{
				nDamage = static_cast<int>(nDamage * TechnoExtData::GetDamageMult(pInvoker, !DamageConsiderVet));

				if (bWarheadDetonate)
				{
					WarheadTypeExtData::DetonateAt(pWarhead, Where, pInvoker, nDamage , pOwner);
				}
				else
				{
					MapClass::DamageArea(Where, nDamage, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
					MapClass::FlashbangWarheadAt(nDamage, pWarhead, Where);
					return { true, nDamage };
				}
			}

			return { false , 0 };
		}

		inline std::pair<bool ,int> Detonate(Nullable<WeaponTypeClass*> const& pWeapon, int nDamage, WarheadTypeClass* pWarhead, bool bWarheadDetonate, const CoordStruct& Where, TechnoClass* pInvoker, HouseClass* pOwner, bool DamageConsiderVet)
		{
			if (!pWeapon.isset()) {
				return DetonateWarhead(nDamage, pWarhead, bWarheadDetonate, Where, pInvoker, pOwner, DamageConsiderVet);
			}

			nDamage = static_cast<int>(nDamage * TechnoExtData::GetDamageMult(pInvoker, !DamageConsiderVet));
			WeaponTypeExtData::DetonateAt(pWeapon, Where, pInvoker, nDamage, false);
			return { false , 0 };
		}

		inline void SpawnMultiple(const std::vector<AnimTypeClass*>& nAnims, std::vector<int>& nAmount, const CoordStruct& Where, TechnoClass* pInvoker, HouseClass* pOwner, bool bRandom)
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
								false
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

		inline std::tuple<bool ,int , int> CheckMinMax(double nMin, double nMax)
		{
			int nMinL = (int)(std::abs(nMin) * 256.0);
			int nMaxL = (int)(std::abs(nMax) * 256.0);

			if (!nMinL && !nMaxL)
				return {false ,0,0};

			if (nMinL > nMaxL)
				std::swap(nMinL, nMaxL);

			return { true ,nMinL,nMaxL };
		}

		inline CoordStruct GetRandomCoordsInsideLoops(double nMin, double nMax, const CoordStruct& nPos, int Increment)
		{
			auto const& [nMinMax, nMinL, nMaxL] = CheckMinMax(nMin, nMax);

			if (nMinMax) {
				auto nRandomCoords = MapClass::GetRandomCoordsNear(nPos,
					(std::abs(ScenarioClass::Instance->Random.RandomRanged(nMinL, nMaxL)) *
					MaxImpl(Increment, 1)),
					ScenarioClass::Instance->Random.RandomBool());

				nRandomCoords.Z = MapClass::Instance->GetCellFloorHeight(nRandomCoords);;
				return nRandomCoords;
			}

			return nPos;
		}
	}
}