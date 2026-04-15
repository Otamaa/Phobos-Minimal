#include "Body.h"

#include <New/Type/CrateTypeClass.h>
#include <New/Entity/FlyingStrings.h>

#include <Misc/DamageArea.h>

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/SWType/Body.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <ExtraHeaders/StackVector.h>

// what is the boolean return for , heh
CollectResult FakeCellClass::_CollecCrate(FootClass* pCollector)
{
	if (pCollector && this->OverlayTypeIndex > -1)
	{
		const auto pOverlay = OverlayTypeClass::Array->Items[this->OverlayTypeIndex];

		if (pOverlay->Crate)
		{
			const auto pCollectorOwner = pCollector->Owner;
			bool force_mcv = false;
			int soloCrateMoney = 0;

			if (SessionClass::Instance->GameMode == GameMode::Campaign || !pCollectorOwner->Type->MultiplayPassive)
			{
				if (pOverlay->CrateTrigger && pCollector->AttachedTag)
				{
					Debug::LogInfo("Springing trigger on crate at {},{}", this->MapCoords.X, this->MapCoords.Y);
					pCollector->AttachedTag->SpringEvent(TriggerEvent::PickupCrate, pCollector, CellStruct::Empty);
					if (!pCollector->IsAlive)
						return CollectResult::cannot;

					ScenarioClass::Instance->PickedUpAnyCrate = true;
				}

				Powerup data = Powerup::Money;

#pragma region DetermineTheRewardType
				if (this->OverlayData < CrateTypeClass::Array.size())
					data = (Powerup)this->OverlayData;
				else
				{
					int total_shares = 0;

					StackVector<Powerup, 256> crates {};

					for (size_t i = 0; i < CrateTypeClass::Array.size(); i++)
					{
						auto crate = CrateTypeClass::Array[i].get();

						if (this->LandType == LandType::Water && !crate->Naval)
						{
							continue;
						}

						if (!this->IsClearToMove(crate->Speed,
							true, true,
							ZoneType::None,
							MovementZone::Normal, -1, true)) continue;

						if (crate->Weight > 0)
						{
							total_shares += crate->Weight;
							crates->push_back((Powerup)i);
						}
					}

					int random = ScenarioClass::Instance->Random.RandomRanged(1, total_shares);
					int share_count = 0;

					for (size_t i = 0; i < crates->size(); i++)
					{
						share_count += CrateTypeClass::Array[(size_t)crates[i]]->Weight;
						if (random <= share_count)
						{
							data = (Powerup)crates[i];
							break;
						}
					}
				}
#pragma endregion

				if (SessionClass::Instance->GameMode != GameMode::Campaign)
				{
					auto pBase = pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->BaseUnit);

					if (GameModeOptionsClass::Instance->Bases
						&& !pCollectorOwner->OwnedBuildings
						&& pCollectorOwner->Available_Money() > RulesExtData::Instance()->FreeMCV_CreditsThreshold
						&& !pCollectorOwner->OwnedUnitTypes.get_count(pBase->ArrayIndex)
						)
					{
						data = Powerup::Unit;
						force_mcv = true;
					}
					const auto landType = this->LandType;

#pragma region EVALUATE_FIST_TIME
					switch ((Powerup)data)
					{
					case Powerup::Unit:
					{
						if (RulesExtData::Instance()->UnitCrateVehicleCap < 0)
							break;

						if (pCollectorOwner->OwnedUnits >= RulesExtData::Instance()->UnitCrateVehicleCap
							|| landType == LandType::Water
							|| landType == LandType::Beach)
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Cloak:
					{

						if (!GET_TECHNOTYPEEXT(pCollector)->CloakAllowed
							|| pCollector->CanICloakByDefault()
							|| TechnoExtContainer::Instance.Find(pCollector)->AE.flags.Cloakable)
							data = Powerup::Money;

						break;
					}
					case Powerup::Squad:
					{
						if (pCollectorOwner->OwnedInfantry > 100
							|| landType == LandType::Water
							|| landType == LandType::Beach)
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Armor:
					{
						if (TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_ArmorMultiplier != 1.0)
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Speed:
					{
						if (TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_SpeedMultiplier != 1.0 || pCollector->WhatAmI() == AbstractType::Aircraft)
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Firepower:
					{
						if (TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_FirepowerMultiplier != 1.0 || !pCollector->IsArmed())
						{
							data = Powerup::Money;
						}

						break;
					}
					case Powerup::Veteran:
					{
						if (!GET_TECHNOTYPE(pCollector)->Trainable || pCollector->Veterancy.IsElite())
						{
							data = Powerup::Money;
						}

						break;
					}
					//both of these are useless for AI , really
					case Powerup::Darkness:
					case Powerup::Reveal:
					{
						if (!pCollectorOwner->IsControlledByHuman())
							data = Powerup::Money;

						break;
					}
					default:
						break;
					}
#pragma endregion

					HouseExtData::IncremetCrateTracking(pCollectorOwner, data);

				}
				else if (!this->OverlayData)
				{
					soloCrateMoney = RulesClass::Instance->SoloCrateMoney;

					if (pOverlay == RulesClass::Instance->CrateImg)
					{
						this->OverlayData = (unsigned char)RulesClass::Instance->SilverCrate;
					}

					if (pOverlay == RulesClass::Instance->WoodCrateImg)
					{
						this->OverlayData = (unsigned char)RulesClass::Instance->WoodCrate;
					}

					if (pOverlay == RulesClass::Instance->WaterCrateImg)
					{
						this->OverlayData = (unsigned char)RulesClass::Instance->WaterCrate;
					}

					data = (Powerup)this->OverlayData;
				}

				MapClass::Instance->Remove_Crate(&this->MapCoords);

				if (SessionClass::Instance->GameMode != GameMode::Campaign && GameModeOptionsClass::Instance->Crates)
				{
					MapClass::Instance->Place_Random_Crate();
				}

#pragma region MainAffect
				const auto something = CrateTypeClass::Array[(int)data]->Argument;
				//not always get used same way ?

				auto PlayAnimAffect = [this, pCollector, pCollectorOwner](Powerup idx)
					{
						if (const auto pAnimType = CrateTypeClass::Array[(int)idx]->Anim)
						{
							auto loc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }) + 200);

							GameCreate<AnimClass>(pAnimType, loc, 0, 1, 0x600, 0, 0);
						}
					};

				auto PlaySoundAffect = [this, pCollector, pCollectorOwner](Powerup idx)
					{
						if (CrateTypeClass::Array[(int)idx]->Sound <= -1)
							return;

						if (pCollectorOwner->ControlledByCurrentPlayer())
						{
							auto loc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }));
							VocClass::SafeImmedietelyPlayAt(CrateTypeClass::Array[(int)idx]->Sound, &loc, nullptr);
						}
					};

				auto GeiveMoney = [&]()
					{

						Debug::LogInfo("Crate at {},{} contains money", this->MapCoords.X, this->MapCoords.Y);

						if (!soloCrateMoney)
						{
							const auto nAdd = RulesExtData::Instance()->RandomCrateMoney;
							int crateMax = 900;

							if (nAdd > 0)
								crateMax += ScenarioClass::Instance->Random.RandomFromMax<int>(nAdd);

							soloCrateMoney = ScenarioClass::Instance->Random.RandomRanged((int)something, (int)something + crateMax);
						}

						const auto pHouseDest = pCollectorOwner->ControlledByCurrentPlayer() || SessionClass::Instance->GameMode != GameMode::Campaign
							? pCollectorOwner : HouseClass::CurrentPlayer();

						pHouseDest->TransactMoney(soloCrateMoney);
						if (pCollectorOwner->ControlledByCurrentPlayer())
						{
							auto loc_fly = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }));
							FlyingStrings::Instance.AddMoneyString(true, soloCrateMoney, pHouseDest, AffectedHouse::Owner, loc_fly, Point2D::Empty, ColorStruct::Empty);
						}
						PlaySoundAffect(Powerup::Money);
						PlayAnimAffect(Powerup::Money);
					};

				switch (data)
				{
				case Powerup::Money:
				{
					GeiveMoney();
					break;
				}
				//TODO :
				// this thing confusing !
				case Powerup::Unit:
				{
					Debug::LogInfo("Crate at {},{} contains a unit", this->MapCoords.X, this->MapCoords.Y);
					UnitTypeClass* Given = nullptr;
					if (force_mcv)
					{
						Given = pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->BaseUnit);
					}

					if (!Given)
					{
						if ((pCollectorOwner->OwnedBuildingTypes.get_count(RulesClass::Instance->BuildRefinery[0]->ArrayIndex) > 0
							|| pCollectorOwner->OwnedBuildingTypes.get_count(RulesClass::Instance->BuildRefinery[1]->ArrayIndex) > 0)
						&& !pCollectorOwner->OwnedUnitTypes.get_count(RulesClass::Instance->HarvesterUnit[0]->ArrayIndex)
						&& !pCollectorOwner->OwnedUnitTypes.get_count(RulesClass::Instance->HarvesterUnit[1]->ArrayIndex)
						)
						{
							Given = pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->HarvesterUnit);
						}
					}

					if (RulesClass::Instance->UnitCrateType)
					{
						Given = RulesClass::Instance->UnitCrateType;
					}

					bool finish = false;
					bool currentPlayer = false;
					if (!Given)
					{
						while (true)
						{
							do
							{
								Given = UnitTypeClass::Array->Items[ScenarioClass::Instance->Random.RandomFromMax(UnitTypeClass::Array->Count - 1)];
								int count = 0;

								if (RulesClass::Instance->BaseUnit.Count > 0)
								{
									auto begin = RulesClass::Instance->BaseUnit.begin();
									while (*begin != Given)
									{
										++begin;
										++count;
										if (count >= RulesClass::Instance->BaseUnit.Count)
										{
											finish = false;
											break;
										}
									}

									finish = true;
								}

								currentPlayer = pCollectorOwner->ControlledByCurrentPlayer();
							}
							while (!Given->CrateGoodie || TechnoTypeExtContainer::Instance.Find(Given)->CrateGoodie_RerollChance > 0.0
								&& TechnoTypeExtContainer::Instance.Find(Given)->CrateGoodie_RerollChance < ScenarioClass::Instance->Random.RandomDouble());

							if (GameModeOptionsClass::Instance->Bases)
								break;

							if (!finish)
								break;
						}(finish && !currentPlayer && !force_mcv);
					}

					if (Given)
					{
						if (auto pCreatedUnit = Given->CreateObject(pCollectorOwner))
						{
							auto loc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }));
							if (pCreatedUnit->Unlimbo(loc, DirType::Min))
							{
								PlaySoundAffect(Powerup::Unit);
								return CollectResult::cannot;
							}

							auto alternative_loc = MapClass::Instance->NearByLocation(this->MapCoords, Given->SpeedType, ZoneType::None
											, Given->MovementZone, 0, 1, 1, 0, 0, 0, 1, CellStruct::Empty, false, false);

							if (alternative_loc.IsValid())
							{
								if (pCreatedUnit->Unlimbo(CellClass::Cell2Coord(alternative_loc), DirType::Min))
								{
									PlaySoundAffect(Powerup::Unit);
									return CollectResult::cannot;
								}
							}

							GameDelete<true, false>(pCreatedUnit);
							GeiveMoney();
							break;
						}
						else
						{
							PlayAnimAffect(Powerup::Unit);
							return CollectResult::can;
						}
					}
				}
				case Powerup::HealBase:
				{
					Debug::LogInfo("Crate at {},{} contains base healing", this->MapCoords.X, this->MapCoords.Y);
					PlaySoundAffect(Powerup::HealBase);
					for (int i = 0; i < MapClass::Logics->Count; ++i)
					{
						if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::Logics->Items[i]))
						{
							if (pTechno->IsAlive && pTechno->GetOwningHouse() == pCollectorOwner)
							{
								int heal = pTechno->Health - GET_TECHNOTYPE(pTechno)->Strength;
								pTechno->ReceiveDamage(&heal, 0, RulesClass::Instance->C4Warhead, 0, 1, 1, nullptr);
							}
						}
					}
					PlayAnimAffect(Powerup::HealBase);
					break;
				}
				case Powerup::Explosion:
				{
					Debug::LogInfo("Crate at {},{} contains explosives", this->MapCoords.X, this->MapCoords.Y);
					int damage = (int)something;
					pCollector->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);
					for (int i = 5; i > 0; --i)
					{
						int scatterDistance = ScenarioClass::Instance->Random.RandomFromMax(512);
						auto loc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }));
						auto randomCoords = MapClass::GetRandomCoordsNear(loc, scatterDistance, false);
						DamageArea::Apply(&randomCoords, damage, nullptr, RulesClass::Instance->C4Warhead, true, nullptr);
						if (auto pAnim = MapClass::SelectDamageAnimation(damage, RulesClass::Instance->C4Warhead, LandType::Clear, randomCoords))
						{
							GameCreate<AnimClass>(pAnim, randomCoords, 0, 1, 0x2600, -15, false);
						}
						MapClass::FlashbangWarheadAt(damage, RulesClass::Instance->C4Warhead, randomCoords);
					}
					PlayAnimAffect(Powerup::Explosion);
					break;
				}
				case Powerup::Napalm:
				{
					Debug::LogInfo("Crate at {},{} contains napalm", this->MapCoords.X, this->MapCoords.Y);
					auto loc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }));
					auto Collector_loc = (pCollector->GetCoords() + loc) / 2;

					GameCreate<AnimClass>(AnimTypeClass::Array->Items[0], Collector_loc, 0, 1, 0x600, 0, 0);
					int damage = (int)something;
					pCollector->ReceiveDamage(&damage, 0, RulesClass::Instance->FlameDamage, nullptr, 1, false, 0);
					DamageArea::Apply(&Collector_loc, damage, nullptr, RulesClass::Instance->FlameDamage, true, nullptr);

					PlayAnimAffect(Powerup::Napalm);
					return CollectResult::can;
				}
				case Powerup::Darkness:
				{
					Debug::LogInfo("Crate at {},{} contains 'shroud'", this->MapCoords.X, this->MapCoords.Y);
					MapClass::Instance->Reshroud(pCollectorOwner);
					PlayAnimAffect(Powerup::Darkness);
					break;
				}
				case Powerup::Reveal:
				{
					Debug::LogInfo("Crate at {},{} contains 'reveal'", this->MapCoords.X, this->MapCoords.Y);
					MapClass::Instance->Reveal(pCollectorOwner->IsControlledByHuman() ? HouseClass::CurrentPlayer : pCollectorOwner);
					PlaySoundAffect(Powerup::Reveal);
					PlayAnimAffect(Powerup::Reveal);
					break;
				}
				case Powerup::Armor:
				{
					Debug::LogInfo("Crate at {},{} contains armor", this->MapCoords.X, this->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;
								if ((int)place.Length() < RulesClass::Instance->CrateRadius && TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_ArmorMultiplier == 1.0)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_ArmorMultiplier = something;
									AEProperties::Recalculate(pCollector);

									if (pTechno->Owner->ControlledByCurrentPlayer())
									{
										VoxClass::Play(GameStrings::EVA_UnitArmorUpgraded());
									}
								}
							}
						}
					}

					PlaySoundAffect(Powerup::Armor);
					PlayAnimAffect(Powerup::Armor);
					break;
				}
				case Powerup::Speed:
				{
					Debug::LogInfo("Crate at {},{} contains speed", this->MapCoords.X, this->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = flag_cast_to<FootClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive && pTechno->WhatAmI() != AbstractType::Aircraft)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;
								if ((int)place.Length() < RulesClass::Instance->CrateRadius && TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_SpeedMultiplier == 1.0)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_SpeedMultiplier = something;
									AEProperties::Recalculate(pCollector);

									if (pTechno->Owner->ControlledByCurrentPlayer())
									{
										VoxClass::Play(GameStrings::EVA_UnitArmorUpgraded());
									}
								}
							}
						}
					}

					PlaySoundAffect(Powerup::Speed);
					PlayAnimAffect(Powerup::Speed);
					break;
				}
				case Powerup::Firepower:
				{
					Debug::LogInfo("Crate at {},{} contains firepower", this->MapCoords.X, this->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;
								if ((int)place.Length() < RulesClass::Instance->CrateRadius
									&& TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_FirepowerMultiplier == 1.0)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_FirepowerMultiplier = something;
									AEProperties::Recalculate(pCollector);

									if (pTechno->Owner->ControlledByCurrentPlayer())
									{
										VoxClass::Play(GameStrings::EVA_UnitFirePowerUpgraded());
									}
								}
							}
						}
					}

					PlaySoundAffect(Powerup::Firepower);
					PlayAnimAffect(Powerup::Firepower);
					break;
				}
				case Powerup::Cloak:
				{
					Debug::LogInfo("Crate at {},{} contains cloaking device", this->MapCoords.X, this->MapCoords.Y);

					for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
					{
						if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
						{
							if (pTechno->IsAlive && pTechno->IsOnMap)
							{
								auto LayersCoords = pTechno->GetCoords();
								auto cellLoc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }));
								auto place = cellLoc - LayersCoords;

								if ((int)place.Length() < RulesClass::Instance->CrateRadius)
								{
									TechnoExtContainer::Instance.Find(pCollector)->AE.flags.Cloakable = true;
									AEProperties::Recalculate(pCollector);
								}
							}
						}
					}

					PlayAnimAffect(Powerup::Cloak);
					break;
				}
				case Powerup::ICBM:
				{
					Debug::LogInfo("Crate at {},{} contains ICBM", this->MapCoords.X, this->MapCoords.Y);

					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
					{
						return pSuper->Type->Type == SuperWeaponType::Nuke && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
					});

					if (iter != pCollectorOwner->Supers.end())
					{
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
						{
							SidebarClass::Instance->AddCameo(AbstractType::Special, (*iter)->Type->ArrayIndex);
						}
					}

					PlayAnimAffect(Powerup::ICBM);
					return CollectResult::can;
				}
				case Powerup::Veteran:
				{
					Debug::LogInfo("Crate at {},{} contains veterancy(TM)", this->MapCoords.X, this->MapCoords.Y);
					const int MaxPromotedCount = (int)something;

					if (MaxPromotedCount > 0)
					{
						for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
						{
							if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]))
							{
								if (pTechno->IsAlive && pTechno->IsOnMap && GET_TECHNOTYPE(pTechno)->Trainable)
								{
									auto LayersCoords = pTechno->GetCoords();
									auto cellLoc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128,128 }));
									auto place = cellLoc - LayersCoords;

									if ((int)place.Length() < RulesClass::Instance->CrateRadius)
									{
										int PromotedCount = 0;
										if (MaxPromotedCount > 0.0)
										{
											do
											{
												if (pTechno->Veterancy.IsVeteran())
													pTechno->Veterancy.SetElite();
												else
													if (pTechno->Veterancy.IsRookie())
														pTechno->Veterancy.SetVeteran();
													else
														if (pTechno->Veterancy.IsNegative())
															pTechno->Veterancy.SetRookie();

												++PromotedCount;
											}
											while ((double)PromotedCount < MaxPromotedCount);
										}
									}
								}
							}
						}
					}

					PlaySoundAffect(Powerup::Veteran);
					PlayAnimAffect(Powerup::Veteran);
					break;
				}
				case Powerup::Gas:
				{
					Debug::LogInfo("Crate at {},{} contains poison gas", this->MapCoords.X, this->MapCoords.Y);

					if (auto WH = WarheadTypeClass::Array->get_or_default(WarheadTypeClass::FindIndexById("GAS")))
					{

						bool randomizeCoord = true;
						auto collector_loc = this->GetCoords();

						DamageArea::Apply(&collector_loc, (int)something, nullptr, WH, true, nullptr);

						for (int i = 0; i < 8;)
						{
							CellClass* pDestCell = this;
							if (randomizeCoord)
							{
								CellStruct dest {};
								MapClass::GetAdjacentCell(&dest, &this->MapCoords, (FacingType)i);
								pDestCell = MapClass::Instance->GetCellAt(dest);
							}

							auto damagearea = pDestCell->GetCoords();
							DamageArea::Apply(&damagearea, (int)something, nullptr, WH, true, nullptr);
							randomizeCoord = ++i < 8;
						}
					}

					PlaySoundAffect(Powerup::Gas);
					PlayAnimAffect(Powerup::Gas);
					break;
				}
				case Powerup::Tiberium:
				{
					Debug::LogInfo("Crate at {},{} contains tiberium", this->MapCoords.X, this->MapCoords.Y);
					int tibToSpawn = ScenarioClass::Instance->Random.RandomFromMax(TiberiumClass::Array->Count - 1);
					if (tibToSpawn == 1)
						tibToSpawn = 0;

					this->IncreaseTiberium(tibToSpawn, 1);

					for (int i = ScenarioClass::Instance->Random.RandomRanged(10, 20); i > 0; --i)
					{
						int distance = ScenarioClass::Instance->Random.RandomFromMax(300);
						auto center = this->GetCoords();
						auto destLoc = MapClass::GetRandomCoordsNear(center, distance, true);
						MapClass::Instance->GetCellAt(destLoc)->IncreaseTiberium(tibToSpawn, 1);
					}

					PlayAnimAffect(Powerup::Tiberium);
					break;
				}
				case Powerup::Squad:
				{
					Debug::LogInfo("Crate at {},{} contains Squad", this->MapCoords.X, this->MapCoords.Y);

					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
 {
	 return pSuper->Type->Type == SuperWeaponType::AmerParaDrop && !pSuper->Granted && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
					});

					if (iter != pCollectorOwner->Supers.end())
					{
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
						{
							SidebarClass::Instance->AddCameo(AbstractType::Special, (*iter)->Type->ArrayIndex);
						}
					}
					else
					{
						GeiveMoney();
						break;
					}

					PlayAnimAffect(Powerup::Squad);
					break;
				}
				case Powerup::Invulnerability:
				{
					Debug::LogInfo("Crate at {},{} contains Invulnerability", this->MapCoords.X, this->MapCoords.Y);
					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
					{
						return pSuper->Type->Type == SuperWeaponType::IronCurtain && !pSuper->Granted && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
					});

					if (iter != pCollectorOwner->Supers.end())
					{
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
						{
							SidebarClass::Instance->AddCameo(AbstractType::Special, (*iter)->Type->ArrayIndex);
						}
					}

					PlayAnimAffect(Powerup::Invulnerability);
					break;
				}
				case Powerup::IonStorm:
				{
					Debug::LogInfo("Crate at {},{} contains IonStorm", this->MapCoords.X, this->MapCoords.Y);
					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
					{
						return pSuper->Type->Type == SuperWeaponType::LightningStorm && !pSuper->Granted && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
					});

					if (iter != pCollectorOwner->Supers.end())
					{
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
						{
							SidebarClass::Instance->AddCameo(AbstractType::Special, (*iter)->Type->ArrayIndex);
						}
					}

					PlayAnimAffect(Powerup::IonStorm);
					break;
				}
				case Powerup::Pod:
				{
					Debug::LogInfo("Crate at {},{} contains Pod", this->MapCoords.X, this->MapCoords.Y);
					auto iter = pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
 {
	 return (NewSuperType)pSuper->Type->Type == NewSuperType::DropPod && !pSuper->Granted && SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
					});

					if (iter != pCollectorOwner->Supers.end())
					{
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
						{
							SidebarClass::Instance->AddCameo(AbstractType::Special, (*iter)->Type->ArrayIndex);
						}
					}

					PlayAnimAffect(Powerup::Pod);
					return CollectResult::can;
				}
				default:
					//TODO :: the affects
					Debug::LogInfo("Crate at {},{} contains {}", this->MapCoords.X, this->MapCoords.Y, CrateTypeClass::Array[(int)data]->Name.data());
					PlaySoundAffect(data);
					PlayAnimAffect(data);
					break;
				}
#pragma endregion
			}
		}
	}

	return CollectResult::can;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x481A00, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x4B0D1B, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x4B0E88, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x4B1DBE, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x4B405D, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x4B46E6, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x5153E9, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x54C9F6, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x5B1894, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x6A03EB, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x6A0558, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x6A1401, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x6A3689, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x6A3D15, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x71972E, FakeCellClass::_CollecCrate)
DEFINE_FUNCTION_JUMP(CALL, 0x75C56C, FakeCellClass::_CollecCrate)

ASMJIT_PATCH(0x56BFC2, MapClass_PlaceCrate_MaxVal, 0x5)
{
	return R->EDX<int>() != (int)CrateTypeClass::Array.size()
		? 0x56BFC7 : 0x56BFFF;
}

ASMJIT_PATCH(0x475A44, CCINIClass_Put_CrateType, 0x7)
{
	GET_STACK(int, crateType, 0x8);

	const auto pCrate = CrateTypeClass::FindFromIndexFix(crateType);
	if (!pCrate)
	{
		Debug::FatalErrorAndExit(__FUNCTION__" Missing CrateType Pointer for[%d]!", crateType);
	}

	R->EDX(pCrate->Name.data());
	return 0x475A4B;
}
ASMJIT_PATCH(0x475A1F, RulesClass_Put_CrateType, 0x5)
{
	GET(const char*, crate, ECX);

	const int idx = CrateTypeClass::FindIndexById(crate);
	if (idx <= -1)
	{
		Debug::FatalErrorAndExit(__FUNCTION__" Missing CrateType index for[%s]!", crate);
	}
	R->EAX(idx);
	return 0x475A24;
}

ASMJIT_PATCH(0x48DE79, CrateTypeFromName, 0x7)
{
	GET(const char*, readedName, EBX);

	const auto type = CrateTypeClass::FindIndexById(readedName);

	if (type != -1)
	{
		R->EDI(type);
		return 0x48DEA2;
	}

	return 0x48DE9C;
}