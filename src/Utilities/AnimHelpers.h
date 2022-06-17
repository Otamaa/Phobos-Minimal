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
				auto nIndexR = (splash.size() - 1);
				auto nIndex = Random ?
					ScenarioClass::Instance->Random.RandomRanged(0, nIndexR) : nIndexR;

				return splash.at(nIndex);
			}

			return nullptr;
		}

		inline void Detonate(Nullable<WeaponTypeClass*> const& pWeapon, int nDamage, WarheadTypeClass* pWarhead, bool bWarheadDetonate, const CoordStruct& Where, TechnoClass* pInvoker, HouseClass* pOwner , bool DamageConsiderVet)
		{
			nDamage = static_cast<int>(nDamage * TechnoExt::GetDamageMult(pInvoker, !DamageConsiderVet));

			if (pWeapon.isset())
			{
				if (BulletClass* pBullet = pWeapon->Projectile->CreateBullet(Map[Where], pInvoker,
					nDamage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright || pWeapon->Warhead->Bright))
				{
					pBullet->SetWeaponType(pWeapon.Get());
					pBullet->Limbo();
					pBullet->SetLocation(Where);
					pBullet->Explode(true);
					pBullet->UnInit();
				}
			}
			else if (pWarhead)
			{
				if (bWarheadDetonate)
				{
					WarheadTypeExt::DetonateAt(pWarhead, Where, pInvoker, nDamage);
				}
				else
				{
					MapClass::DamageArea(Where, nDamage, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
					MapClass::FlashbangWarheadAt(nDamage, pWarhead, Where);
				}
			}
		}

		inline void Detonate(int nDamage, WarheadTypeClass* pWarhead, bool bWarheadDetonate, const CoordStruct& Where, TechnoClass* pInvoker, HouseClass* pOwner, bool DamageConsiderVet)
		{
			if (pWarhead)
			{
				nDamage = static_cast<int>(nDamage * TechnoExt::GetDamageMult(pInvoker, !DamageConsiderVet));

				if (bWarheadDetonate) {
					WarheadTypeExt::DetonateAt(pWarhead, Where, pInvoker, nDamage);
				}
				else {
					MapClass::DamageArea(Where, nDamage, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
					MapClass::FlashbangWarheadAt(nDamage, pWarhead, Where);
				}
			}
		}

		inline void SpawnMultiple(const std::vector<AnimTypeClass*>& nAnims, DynamicVectorClass<int>& nAmount, const CoordStruct& Where, TechnoClass* pInvoker, HouseClass* pOwner, bool bRandom)
		{
			if (!nAnims.empty())
			{
				auto nCreateAnim = [&](int nIndex)
				{
					if (auto const pMultipleSelected = nAnims[nIndex])
					{
						if ((size_t)nAmount[nIndex] > 0)
						{
							for (size_t k = (size_t)nAmount[nIndex]; k > 0; --k)
							{
								if (auto pAnimCreated = GameCreate<AnimClass>(pMultipleSelected, Where))
								{
									AnimExt::SetAnimOwnerHouseKind(pAnimCreated, pOwner, nullptr, false);
									if (auto const pAnimExt = AnimExt::GetExtData(pAnimCreated))
										pAnimExt->Invoker = pInvoker;
								}
							}
						}
					}
				};

				if (!bRandom)
				{
					for (size_t i = 0; i < nAnims.size(); i++)
					{
						nCreateAnim(i);
					}
				}
				else
				{
					nCreateAnim(ScenarioGlobal->Random.RandomRanged(0, nAnims.size() - 1));
				}
			}
		}

		inline bool CheckMinMax(double nMin, double nMax, int& nOutMin, int& nOutMax)
		{
			int nMinL = Game::F2I(abs(nMin) * 256.0);
			int nMaxL = Game::F2I(abs(nMax) * 256.0);

			if (!nMinL && !nMaxL)
				return false;

			if (nMinL > nMaxL)
				std::swap(nMinL, nMaxL);

			nOutMin = nMinL;
			nOutMax = nMaxL;

			return true;
		}

		inline CoordStruct GetRandomCoordsInsideLoops(double nMin, double nMax, const CoordStruct& nPos, int Increment)
		{
			CoordStruct nBuff = nPos;
			int nMinL = 0;
			int nMaxL = 0;

			if (CheckMinMax(nMin, nMax, nMinL, nMaxL))
			{
				auto nRandomCoords = MapClass::GetRandomCoordsNear(nPos,
					(abs(ScenarioClass::Instance->Random.RandomRanged(nMinL, nMaxL)) * Math::min(Increment, 1)),
					ScenarioClass::Instance->Random.RandomRanged(0, 1));

				nRandomCoords.Z = MapClass::Instance->GetCellFloorHeight(nRandomCoords);;
				nBuff = nRandomCoords;
			}

			return nBuff;
		}
	}
}