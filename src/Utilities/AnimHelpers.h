#pragma once

#include <Ext/WarheadType/Body.h>
#include <Ext/Techno/Body.h>
#include "TemplateDef.h"

namespace Helper
{
	namespace Otamaa
	{
		inline AnimTypeClass* PickSplashAnim(NullableVector<AnimTypeClass*> const& nSplash, AnimTypeClass* pWake, bool Random, bool IsMeteor)
		{
			TypeList<AnimTypeClass*> defaultSplashAnims;

			if (!IsMeteor)
			{
				defaultSplashAnims = TypeList<AnimTypeClass*>();
				defaultSplashAnims.AddItem(pWake);
			}
			else
			{
				defaultSplashAnims = RulesClass::Instance->SplashList;
			}

			auto const splash = nSplash.GetElements(defaultSplashAnims);

			if (splash.size() > 0)
			{
				const auto nIndexR = (splash.size() - 1);
				return splash[Random ?
					ScenarioClass::Instance->Random.RandomFromMax(nIndexR) : nIndexR];
			}

			return nullptr;
		}

		inline std::pair<bool, int> DetonateWarhead(int nDamage, WarheadTypeClass* pWarhead, bool bWarheadDetonate, const CoordStruct& Where, TechnoClass* pInvoker, HouseClass* pOwner, bool DamageConsiderVet)
		{
			if (pWarhead)
			{
				nDamage = static_cast<int>(nDamage * TechnoExt::GetDamageMult(pInvoker, !DamageConsiderVet));

				if (bWarheadDetonate)
				{
					WarheadTypeExt::DetonateAt(pWarhead, Where, pInvoker, nDamage);
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

			nDamage = static_cast<int>(nDamage * TechnoExt::GetDamageMult(pInvoker, !DamageConsiderVet));
			WeaponTypeExt::DetonateAt(pWeapon, Where, pInvoker, nDamage);
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
							if (auto pAnimCreated = GameCreate<AnimClass>(pMultipleSelected, Where)) {
								AnimExt::SetAnimOwnerHouseKind(pAnimCreated, pOwner, nullptr, pInvoker, false) ;
							}
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
			int nMinL = Game::F2I(abs(nMin) * 256.0);
			int nMaxL = Game::F2I(abs(nMax) * 256.0);

			if (!nMinL && !nMaxL)
				return {false ,0,0};

			if (nMinL > nMaxL)
				std::swap(nMinL, nMaxL);

			return { true ,nMinL,nMaxL };
		}

		inline CoordStruct GetRandomCoordsInsideLoops(double nMin, double nMax, const CoordStruct& nPos, int Increment)
		{
			auto const [nMinMax, nMinL, nMaxL] = CheckMinMax(nMin, nMax);

			if (nMinMax) {
				auto nRandomCoords = MapClass::GetRandomCoordsNear(nPos,
					(abs(ScenarioClass::Instance->Random.RandomRanged(nMinL, nMaxL)) * Math::min(Increment, 1)),
					ScenarioClass::Instance->Random.RandomBool());

				nRandomCoords.Z = MapClass::Instance->GetCellFloorHeight(nRandomCoords);;
				return nRandomCoords;
			}

			return nPos;
		}
	}
}