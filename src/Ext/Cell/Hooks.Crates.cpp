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

#ifdef _Old
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
						&& pCollectorOwner->Available_Money() > FakeRulesClass::Instance()->FreeMCV_CreditsThreshold
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
						if (FakeRulesClass::Instance()->UnitCrateVehicleCap < 0)
							break;

						if (pCollectorOwner->OwnedUnits >= FakeRulesClass::Instance()->UnitCrateVehicleCap
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
							const auto nAdd = FakeRulesClass::Instance()->RandomCrateMoney;
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
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer) {
							SidebarClass::Instance->AddSpecialCameo((*iter)->Type->ArrayIndex);
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
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer) {
							SidebarClass::Instance->AddSpecialCameo((*iter)->Type->ArrayIndex);
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
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer) {
							SidebarClass::Instance->AddSpecialCameo((*iter)->Type->ArrayIndex);
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
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer) {
							SidebarClass::Instance->AddSpecialCameo((*iter)->Type->ArrayIndex);
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
						if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer) {
							SidebarClass::Instance->AddSpecialCameo((*iter)->Type->ArrayIndex);
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
#else
// CrateContext bundles the values every crate-effect helper needs, so we
// don't pass 6+ args around or recompute the same loc/cellLoc repeatedly.
struct CrateContext
{
	FakeCellClass* pCell;
	FootClass* pCollector;
	HouseClass* pCollectorOwner;
	CoordStruct      loc;       // CellClass::Cell2Coord at floor height + 200 (anim height)
	CoordStruct      locSound;  // CellClass::Cell2Coord at floor height (sound/effect height)
	Powerup          data;      // valid Powerup only if pType->ArrayIndex < Powerups::Effects.size()
	CrateTypeClass* pType;      // the actual resolved crate type (vanilla OR custom)
	double           something; // pType->Argument
	bool isControlledByPlayer;

	// True if this crate type corresponds to a real Powerup enumerator
	// (i.e. it's one of the vanilla Powerups::Effects entries).
	bool IsVanillaPowerup() const;
};

// Picks the crate reward type (weighted random / fixed OverlayData).
Powerup Crate_DetermineRewardType(FakeCellClass* pCell, FootClass* pCollector);

// Multiplayer-only sanity pass that may downgrade `data` to Money if the
// chosen reward doesn't make sense for the collector (e.g. already cloaked).
Powerup Crate_EvaluateMultiplayerReward(Powerup data, FootClass* pCollector,
	HouseClass* pCollectorOwner, LandType landType, bool& force_mcv);

// Campaign-only: resolves fixed solo-crate overlay types (silver/wood/water)
// and sets soloCrateMoney from RulesClass::SoloCrateMoney.
Powerup Crate_ResolveCampaignReward(FakeCellClass* pCell, int& soloCrateMoney);

// Grants money to the collector (handles solo random range + flying string).
void Crate_GiveMoney(const CrateContext& ctx, int& soloCrateMoney, bool isForwarded = true);

// One handler per Powerup effect. Each returns the CollectResult that the
// caller (_CollecCrate) should propagate. CollectResult::can means
// "fall through to default visuals are already handled inside"; the caller
// just returns what's given. Handlers that want the default switch fallthrough
// behavior (Unit -> HealBase historically) are merged explicitly where needed.
CollectResult Crate_Handle_Unit(CrateContext& ctx, int& soloCrateMoney);
CollectResult Crate_Handle_HealBase(const CrateContext& ctx);
CollectResult Crate_Handle_Explosion(const CrateContext& ctx);
CollectResult Crate_Handle_Napalm(const CrateContext& ctx);
CollectResult Crate_Handle_Darkness(const CrateContext& ctx);
CollectResult Crate_Handle_Reveal(const CrateContext& ctx);
CollectResult Crate_Handle_Armor(const CrateContext& ctx);
CollectResult Crate_Handle_Speed(const CrateContext& ctx);
CollectResult Crate_Handle_Firepower(const CrateContext& ctx);
CollectResult Crate_Handle_Cloak(const CrateContext& ctx);
CollectResult Crate_Handle_ICBM(const CrateContext& ctx);
CollectResult Crate_Handle_Veteran(const CrateContext& ctx);
CollectResult Crate_Handle_Gas(const CrateContext& ctx);
CollectResult Crate_Handle_Tiberium(const CrateContext& ctx);
CollectResult Crate_Handle_Squad(CrateContext& ctx, int& soloCrateMoney);
CollectResult Crate_Handle_Invulnerability(const CrateContext& ctx);
CollectResult Crate_Handle_IonStorm(const CrateContext& ctx);
CollectResult Crate_Handle_Pod(const CrateContext& ctx);
CollectResult Crate_Handle_Default(const CrateContext& ctx);

// Generic helper: finds the first non-granted super of `type` whose
// SWTypeExt has CrateGoodies set, grants it, and shows the cameo if the
// collector belongs to the current player. Returns true if granted.
bool Crate_GrantSuperOfType(HouseClass* pOwner, FootClass* pCollector, SuperWeaponType type);

// Same as above but for a NewSuperType (e.g. DropPod).
bool Crate_GrantNewSuperOfType(HouseClass* pOwner, FootClass* pCollector, NewSuperType type);

// ---------------------------------------------------------------------
// Custom crate type dispatch
// ---------------------------------------------------------------------
//
// CrateTypeClass::Array entries with ArrayIndex >= Powerups::Effects.size()
// are modder-defined ("custom") crate types that don't correspond to any
// real Powerup enumerator. Their effect is selected by SECTION NAME, not
// by Powerup index. Register a handler for a given section name with
// Crate_RegisterCustomHandler(); Crate_Dispatch will look it up by name
// when ctx.data falls outside the vanilla Powerup range.

using CrateCustomHandler = CollectResult(*)(CrateContext& ctx, int& soloCrateMoney);

// Call during init (e.g. after CrateTypeClass::ReadFromINIList) to bind a
// handler to a [CrateTypes] section name.
void Crate_RegisterCustomHandler(const char* sectionName, CrateCustomHandler handler);

// Looks up a registered handler by the crate type's Name. Returns nullptr
// if none registered (caller should fall back to Crate_Handle_Default).
CrateCustomHandler Crate_FindCustomHandler(const char* sectionName);

// Top-level entry point used by _CollecCrate. Handles the vanilla/custom
// split internally:
//  - vanilla Powerup        -> Crate_Dispatch (switch on ctx.data)
//  - custom type, registered -> the registered CrateCustomHandler
//  - custom type, unregistered -> Crate_Handle_Default (Anim/Sound only)
CollectResult Crate_HandleCrate(CrateContext& ctx, int& soloCrateMoney);

// ---------------------------------------------------------------------
// CrateContext
// ---------------------------------------------------------------------

bool CrateContext::IsVanillaPowerup() const
{
	return (size_t)this->pType->ArrayIndex < Powerups::Effects.size();
}

// ---------------------------------------------------------------------
// Reward type determination
// ---------------------------------------------------------------------

Powerup Crate_DetermineRewardType(FakeCellClass* pCell, FootClass* pCollector)
{
	if (pCell->OverlayData < CrateTypeClass::Array.size())
		return (Powerup)pCell->OverlayData;

	int total_shares = 0;
	StackVector<Powerup, 19> crates {};

	for (size_t i = 0; i < CrateTypeClass::Array.size(); i++)
	{
		auto crate = CrateTypeClass::Array[i].get();

		if (pCell->LandType == LandType::Water && !crate->Naval)
			continue;

		if (!pCell->IsClearToMove(crate->Speed, true, true, ZoneType::None, MovementZone::Normal, -1, true))
			continue;

		if (crate->Weight > 0)
		{
			total_shares += crate->Weight;
			crates->push_back((Powerup)i);
		}
	}

	if (total_shares <= 0 || crates->size() == 0)
		return Powerup::Money; // VERIFY: vanilla fallback when nothing is eligible

	const int random = ScenarioClass::Instance->Random.RandomRanged(1, total_shares);
	int share_count = 0;

	for (size_t i = 0; i < crates->size(); i++)
	{
		share_count += CrateTypeClass::Array[(size_t)crates[i]]->Weight;
		if (random <= share_count)
			return crates[i];
	}

	return Powerup::Money;
}

// ---------------------------------------------------------------------
// Multiplayer reward sanity pass
// ---------------------------------------------------------------------

Powerup Crate_EvaluateMultiplayerReward(Powerup data, FootClass* pCollector,
	HouseClass* pCollectorOwner, LandType landType, bool& force_mcv)
{
	const auto pBase = pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->BaseUnit);

	if (GameModeOptionsClass::Instance->Bases
		&& !pCollectorOwner->OwnedBuildings
		&& pCollectorOwner->Available_Money() > FakeRulesClass::Instance()->FreeMCV_CreditsThreshold
		&& !pCollectorOwner->OwnedUnitTypes.get_count(pBase->ArrayIndex))
	{
		data = Powerup::Unit;
		force_mcv = true;
	}

	switch (data)
	{
	case Powerup::Unit:
	{
		if (FakeRulesClass::Instance()->UnitCrateVehicleCap < 0)
			break;

		if (pCollectorOwner->OwnedUnits >= FakeRulesClass::Instance()->UnitCrateVehicleCap
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
		{
			data = Powerup::Money;
		}
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
			data = Powerup::Money;
		break;
	}
	case Powerup::Speed:
	{
		if (TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_SpeedMultiplier != 1.0
			|| pCollector->WhatAmI() == AbstractType::Aircraft)
		{
			data = Powerup::Money;
		}
		break;
	}
	case Powerup::Firepower:
	{
		if (TechnoExtContainer::Instance.Find(pCollector)->AE.Crate_FirepowerMultiplier != 1.0
			|| !pCollector->IsArmed())
		{
			data = Powerup::Money;
		}
		break;
	}
	case Powerup::Veteran:
	{
		if (!GET_TECHNOTYPE(pCollector)->Trainable || pCollector->Veterancy.IsElite())
			data = Powerup::Money;
		break;
	}
	// both of these are useless for AI, really
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

	return data;
}

// ---------------------------------------------------------------------
// Campaign solo crate resolution
// ---------------------------------------------------------------------

Powerup Crate_ResolveCampaignReward(FakeCellClass* pCell, int& soloCrateMoney)
{
	if (pCell->OverlayData)
		return (Powerup)pCell->OverlayData;

	soloCrateMoney = RulesClass::Instance->SoloCrateMoney;

	const auto pOverlay = OverlayTypeClass::Array->Items[pCell->OverlayTypeIndex];

	if (pOverlay == RulesClass::Instance->CrateImg)
		pCell->OverlayData = (unsigned char)RulesClass::Instance->SilverCrate;
	else if (pOverlay == RulesClass::Instance->WoodCrateImg)
		pCell->OverlayData = (unsigned char)RulesClass::Instance->WoodCrate;
	else if (pOverlay == RulesClass::Instance->WaterCrateImg)
		pCell->OverlayData = (unsigned char)RulesClass::Instance->WaterCrate;

	return (Powerup)pCell->OverlayData;
}

// ---------------------------------------------------------------------
// Money
// ---------------------------------------------------------------------

void Crate_GiveMoney(const CrateContext& ctx, int& soloCrateMoney, bool isForwarded)
{
	// GiveMoney is not called from original result 
	// so some the affect is pulled direcly thru the proper one 
	// also not logging it because the original crate affect already does that 
	// as per vanilla does only the log are different 
	if (!isForwarded)
	   Debug::LogInfo("Crate at {},{} contains money", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	if (!soloCrateMoney)
	{
		const auto nAdd = FakeRulesClass::Instance()->RandomCrateMoney;
		int crateMax = 900;

		if (nAdd > 0)
			crateMax += ScenarioClass::Instance->Random.RandomFromMax<int>(nAdd);

		soloCrateMoney = ScenarioClass::Instance->Random.RandomRanged(
			(int)ctx.something, (int)ctx.something + crateMax);
	}

	const auto pHouseDest =
		ctx.pCollectorOwner->ControlledByCurrentPlayer() || SessionClass::Instance->GameMode != GameMode::Campaign
		? ctx.pCollectorOwner
		: HouseClass::CurrentPlayer();

	pHouseDest->TransactMoney(soloCrateMoney);

	if (ctx.pCollectorOwner->ControlledByCurrentPlayer()) {
		FlyingStrings::Instance.AddMoneyString(true, soloCrateMoney, pHouseDest,
			AffectedHouse::Owner, ctx.locSound, Point2D::Empty, ColorStruct::Empty);
	}

	if (!isForwarded)
		ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	else
		CrateTypeClass::Array[(int)PowerupEffects::Money]->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
}

// ---------------------------------------------------------------------
// Super-weapon crate goodies (ICBM / Squad / Invulnerability / IonStorm / Pod)
// ---------------------------------------------------------------------

bool Crate_GrantSuperOfType(HouseClass* pOwner, FootClass* pCollector, SuperWeaponType type)
{
	const auto iter = pOwner->Supers.find_if([type](SuperClass* pSuper)
	{
		return pSuper->Type->Type == type
			&& !pSuper->Granted
			&& SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
	});

	if (iter == pOwner->Supers.end())
		return false;

	if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
		SidebarClass::Instance->AddSpecialCameo((*iter)->Type->ArrayIndex);

	return true;
}

bool Crate_GrantNewSuperOfType(HouseClass* pOwner, FootClass* pCollector, NewSuperType type)
{
	const auto iter = pOwner->Supers.find_if([type](SuperClass* pSuper)
	{
		return (NewSuperType)pSuper->Type->Type == type
			&& !pSuper->Granted
			&& SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
	});

	if (iter == pOwner->Supers.end())
		return false;

	if ((*iter)->Grant(true, false, false) && pCollector->IsOwnedByCurrentPlayer)
		SidebarClass::Instance->AddSpecialCameo((*iter)->Type->ArrayIndex);

	return true;
}

// ---------------------------------------------------------------------
// Per-powerup handlers
// ---------------------------------------------------------------------

// NOTE: ICBM's SuperWeaponType::Nuke had no "!pSuper->Granted" check in the
// original code (unlike Squad/Invulnerability/IonStorm/Pod), so this preserves
// that asymmetry by NOT using Crate_GrantSuperOfType (which requires !Granted).
CollectResult Crate_Handle_ICBM(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains ICBM", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	const auto iter = ctx.pCollectorOwner->Supers.find_if([](SuperClass* pSuper)
	{
		return pSuper->Type->Type == SuperWeaponType::Nuke
			&& SWTypeExtContainer::Instance.Find(pSuper->Type)->CrateGoodies;
	});

	if (iter != ctx.pCollectorOwner->Supers.end())
	{
		if ((*iter)->Grant(true, false, false) && ctx.pCollector->IsOwnedByCurrentPlayer)
			SidebarClass::Instance->AddSpecialCameo((*iter)->Type->ArrayIndex);
	}

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Squad(CrateContext& ctx, int& soloCrateMoney)
{
	Debug::LogInfo("Crate at {},{} contains Squad", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	if (!Crate_GrantSuperOfType(ctx.pCollectorOwner, ctx.pCollector, SuperWeaponType::AmerParaDrop))
	{
		Crate_GiveMoney(ctx, soloCrateMoney);
		return CollectResult::can;
	}

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Invulnerability(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains Invulnerability", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	Crate_GrantSuperOfType(ctx.pCollectorOwner, ctx.pCollector, SuperWeaponType::IronCurtain);

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_IonStorm(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains IonStorm", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	Crate_GrantSuperOfType(ctx.pCollectorOwner, ctx.pCollector, SuperWeaponType::LightningStorm);

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Pod(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains Pod", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	Crate_GrantNewSuperOfType(ctx.pCollectorOwner, ctx.pCollector, NewSuperType::DropPod);

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_HealBase(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains base healing", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);

	for (int i = 0; i < MapClass::Logics->Count; ++i)
	{
		if (auto pTechno = flag_cast_to<TechnoClass*>(MapClass::Logics->Items[i]))
		{
			if (pTechno->IsAlive && pTechno->GetOwningHouse() == ctx.pCollectorOwner)
			{
				int heal = pTechno->Health - GET_TECHNOTYPE(pTechno)->Strength;
				pTechno->ReceiveDamage(&heal, 0, RulesClass::Instance->C4Warhead, 0, 1, 1, nullptr);
			}
		}
	}

	return CollectResult::can;
}

CollectResult Crate_Handle_Explosion(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains explosives", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	int damage = (int)ctx.something;
	ctx.pCollector->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, 0, 1, 0, 0);

	for (int i = 5; i > 0; --i)
	{
		const int scatterDistance = ScenarioClass::Instance->Random.RandomFromMax(512);
		auto randomCoords = MapClass::GetRandomCoordsNear(ctx.locSound, scatterDistance, false);

		DamageArea::Apply(&randomCoords, damage, nullptr, RulesClass::Instance->C4Warhead, true, nullptr);

		if (auto pAnim = MapClass::SelectDamageAnimation(damage, RulesClass::Instance->C4Warhead, LandType::Clear, randomCoords))
			GameCreate<AnimClass>(pAnim, randomCoords, 0, 1, 0x2600, -15, false);

		MapClass::FlashbangWarheadAt(damage, RulesClass::Instance->C4Warhead, randomCoords);
	}

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Napalm(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains napalm", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	auto collectorLoc = (ctx.pCollector->GetCoords() + ctx.locSound) / 2;

	GameCreate<AnimClass>(AnimTypeClass::Array->Items[0], collectorLoc, 0, 1, 0x600, 0, 0);

	int damage = (int)ctx.something;
	ctx.pCollector->ReceiveDamage(&damage, 0, RulesClass::Instance->FlameDamage, nullptr, 1, false, 0);
	DamageArea::Apply(&collectorLoc, damage, nullptr, RulesClass::Instance->FlameDamage, true, nullptr);

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Darkness(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains 'shroud'", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	MapClass::Instance->Reshroud(ctx.pCollectorOwner);
	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Reveal(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains 'reveal'", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	MapClass::Instance->Reveal(ctx.pCollectorOwner->IsControlledByHuman()
		? HouseClass::CurrentPlayer
		: ctx.pCollectorOwner);

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

// Shared body for Armor / Speed / Firepower: scan layer-2 objects within
// CrateRadius and apply a multiplier exactly once.
template <typename TFilter, typename TApply>
static void Crate_ApplyAreaMultiplier(const CrateContext& ctx, TFilter&& filter, TApply&& apply)
{
	for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
	{
		auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]);
		if (!pTechno || !pTechno->IsAlive)
			continue;

		if (!filter(pTechno))
			continue;

		const auto place = ctx.locSound - pTechno->GetCoords();
		if ((int)place.Length() >= RulesClass::Instance->CrateRadius)
			continue;

		apply(pTechno);
	}
}

CollectResult Crate_Handle_Armor(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains armor", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	Crate_ApplyAreaMultiplier(ctx,
		[](TechnoClass* pTechno) { return true; },
		[&](TechnoClass* pTechno)
		{
			auto pExt = TechnoExtContainer::Instance.Find(pTechno);
			if (pExt->AE.Crate_ArmorMultiplier != 1.0)
				return;

			pExt->AE.Crate_ArmorMultiplier = ctx.something;
			AEProperties::Recalculate(pTechno);
		});

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Speed(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains speed", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	Crate_ApplyAreaMultiplier(ctx,
		[](TechnoClass* pTechno) { return pTechno->WhatAmI() != AbstractType::Aircraft; },
		[&](TechnoClass* pTechno)
		{
			auto pExt = TechnoExtContainer::Instance.Find(pTechno);
			if (pExt->AE.Crate_SpeedMultiplier != 1.0)
				return;

			pExt->AE.Crate_SpeedMultiplier = ctx.something;
			AEProperties::Recalculate(pTechno);

		});

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Firepower(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains firepower", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	Crate_ApplyAreaMultiplier(ctx,
		[](TechnoClass* pTechno) { return true; },
		[&](TechnoClass* pTechno)
		{
			auto pExt = TechnoExtContainer::Instance.Find(pTechno);
			if (pExt->AE.Crate_FirepowerMultiplier != 1.0)
				return;

			pExt->AE.Crate_FirepowerMultiplier = ctx.something;
			AEProperties::Recalculate(pTechno);
		});

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Cloak(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains cloaking device", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
	{
		auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]);
		if (!pTechno || !pTechno->IsAlive || !pTechno->IsOnMap)
			continue;

		const auto place = ctx.locSound - pTechno->GetCoords();
		if ((int)place.Length() < RulesClass::Instance->CrateRadius)
		{
			TechnoExtContainer::Instance.Find(pTechno)->AE.flags.Cloakable = true;
			AEProperties::Recalculate(pTechno);
		}
	}

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Veteran(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains veterancy(TM)", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	const int MaxPromotedCount = (int)ctx.something;

	if (MaxPromotedCount > 0)
	{
		for (int i = 0; i < MapClass::ObjectsInLayers[2].Count; ++i)
		{
			auto pTechno = flag_cast_to<TechnoClass*>(MapClass::ObjectsInLayers[2].Items[i]);
			if (!pTechno || !pTechno->IsAlive || !pTechno->IsOnMap || !GET_TECHNOTYPE(pTechno)->Trainable)
				continue;

			const auto place = ctx.locSound - pTechno->GetCoords();
			if ((int)place.Length() >= RulesClass::Instance->CrateRadius)
				continue;

			// Original loop incremented a counter up to MaxPromotedCount while
			// repeatedly promoting the SAME unit (Rookie->Veteran->Elite, then
			// stuck at Elite for remaining iterations). Preserved as-is below.
			for (int promoted = 0; promoted < MaxPromotedCount; ++promoted)
			{
				if (pTechno->Veterancy.IsVeteran())
					pTechno->Veterancy.SetElite();
				else if (pTechno->Veterancy.IsRookie())
					pTechno->Veterancy.SetVeteran();
				else if (pTechno->Veterancy.IsNegative())
					pTechno->Veterancy.SetRookie();
			}
		}
	}

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Gas(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains poison gas", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	if (auto WH = WarheadTypeClass::Array->get_or_default(WarheadTypeClass::FindIndexById("GAS")))
	{
		const int damage = (int)ctx.something;
		auto collectorCellLoc = ctx.pCell->GetCoords();

		DamageArea::Apply(&collectorCellLoc, damage, nullptr, WH, true, nullptr);

		CellClass* pDestCell = ctx.pCell;
		for (int i = 0; i < 8; ++i)
		{
			CellStruct dest {};
			MapClass::GetAdjacentCell(&dest, &ctx.pCell->MapCoords, (FacingType)i);
			pDestCell = MapClass::Instance->GetCellAt(dest);

			auto damageArea = pDestCell->GetCoords();
			DamageArea::Apply(&damageArea, damage, nullptr, WH, true, nullptr);
		}
	}

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Tiberium(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains tiberium", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	int tibToSpawn = ScenarioClass::Instance->Random.RandomFromMax(TiberiumClass::Array->Count - 1);
	if (tibToSpawn == 1)
		tibToSpawn = 0;

	ctx.pCell->IncreaseTiberium(tibToSpawn, 1);

	for (int i = ScenarioClass::Instance->Random.RandomRanged(10, 20); i > 0; --i)
	{
		const int distance = ScenarioClass::Instance->Random.RandomFromMax(300);
		const auto destLoc = MapClass::GetRandomCoordsNear(ctx.pCell->GetCoords(), distance, true);
		MapClass::Instance->GetCellAt(destLoc)->IncreaseTiberium(tibToSpawn, 1);
	}

	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

CollectResult Crate_Handle_Default(const CrateContext& ctx)
{
	Debug::LogInfo("Crate at {},{} contains {} It has no real affect!", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y,
		ctx.pType->Name.data());
	
	//Only affect are playing here , maybe shouse make proper function affects
	//dunno yet
	ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
	return CollectResult::can;
}

// ---------------------------------------------------------------------
// Custom crate type registry
// ---------------------------------------------------------------------

#include <unordered_map>

static std::unordered_map<std::string, CrateCustomHandler>& Crate_CustomRegistry()
{
	// Function-local static: avoids global init-order issues with other
	// statics (e.g. CrateTypeClass::Array) that might register handlers
	// during their own static init.
	static std::unordered_map<std::string, CrateCustomHandler> registry;
	return registry;
}

void Crate_RegisterCustomHandler(const char* sectionName, CrateCustomHandler handler)
{
	Crate_CustomRegistry()[sectionName] = handler;
}

CrateCustomHandler Crate_FindCustomHandler(const char* sectionName)
{
	auto& registry = Crate_CustomRegistry();
	auto it = registry.find(sectionName);
	return it != registry.end() ? it->second : nullptr;
}

// ---------------------------------------------------------------------
// Top-level dispatch: vanilla Powerup switch vs custom-type registry
// ---------------------------------------------------------------------

// Forward decl of the vanilla switch, defined in FakeCellClass_CollecCrate.cpp
CollectResult Crate_Dispatch(CrateContext& ctx, int& soloCrateMoney);

CollectResult Crate_HandleCrate(CrateContext& ctx, int& soloCrateMoney)
{
	if (ctx.IsVanillaPowerup())
	{
		// ctx.data is a real Powerup enumerator -> normal switch dispatch.
		return Crate_Dispatch(ctx, soloCrateMoney);
	}

	// Custom crate type (ArrayIndex >= Powerups::Effects.size()).
	// Look up by section name; fall back to Anim/Sound-only default.
	if (auto handler = Crate_FindCustomHandler(ctx.pType->Name.data()))
		return handler(ctx, soloCrateMoney);

	return Crate_Handle_Default(ctx);
}

// ---------------------------------------------------------------------
// Unit crate (rewritten retry loop)
// ---------------------------------------------------------------------
//
// Original code had a `while(true)` whose exit conditions never reliably
// terminated and ended with a dangling expression statement
// `(finish && !currentPlayer && !force_mcv);` that did nothing.
// Rewritten as: pick candidates, validate against CrateGoodie / reroll
// chance / BaseUnit list, bounded retry count to avoid infinite loop.

CollectResult Crate_Handle_Unit(CrateContext& ctx, int& soloCrateMoney)
{
	Debug::LogInfo("Crate at {},{} contains a unit", ctx.pCell->MapCoords.X, ctx.pCell->MapCoords.Y);

	UnitTypeClass* Given = nullptr;
	bool force_mcv = false; // VERIFY: caller should pass force_mcv in via ctx if needed elsewhere

	if (force_mcv)
		Given = ctx.pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->BaseUnit);

	if (!Given) {
		const bool hasRefinery =
			ctx.pCollectorOwner->OwnedBuildingTypes.get_count(RulesClass::Instance->BuildRefinery[0]->ArrayIndex) > 0
			|| ctx.pCollectorOwner->OwnedBuildingTypes.get_count(RulesClass::Instance->BuildRefinery[1]->ArrayIndex) > 0;

		const bool hasHarvester =
			ctx.pCollectorOwner->OwnedUnitTypes.get_count(RulesClass::Instance->HarvesterUnit[0]->ArrayIndex) > 0
			|| ctx.pCollectorOwner->OwnedUnitTypes.get_count(RulesClass::Instance->HarvesterUnit[1]->ArrayIndex) > 0;

		if (hasRefinery && !hasHarvester)
			Given = ctx.pCollectorOwner->PickUnitFromTypeList(RulesClass::Instance->HarvesterUnit);
	}

	if (RulesClass::Instance->UnitCrateType)
		Given = RulesClass::Instance->UnitCrateType;

	if (!Given) {
		// Helper: is `type` present in the BaseUnit list?
		auto isBaseUnit = [](UnitTypeClass* type) -> bool {
			if (RulesClass::Instance->BaseUnit.Count <= 0)
				return false;

			for (auto it = RulesClass::Instance->BaseUnit.begin(); it != RulesClass::Instance->BaseUnit.end(); ++it) {
				if (*it == type)
					return true;
			}
			return false;
		};

		constexpr int MaxRerollAttempts = 64; // bounded retry to avoid infinite loop (was unbounded while(true))

		for (int attempt = 0; attempt < MaxRerollAttempts; ++attempt) {
			auto candidate = UnitTypeClass::Array->Items[
				ScenarioClass::Instance->Random.RandomFromMax(UnitTypeClass::Array->Count - 1)];

			if (!candidate->CrateGoodie)
				continue;

			const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(candidate);
			if (pTypeExt->CrateGoodie_RerollChance > 0.0
				&& pTypeExt->CrateGoodie_RerollChance < ScenarioClass::Instance->Random.RandomDouble()) {
				continue;
			}

			// VERIFY: original logic excluded BaseUnit-list units for AI/non-current-player
			// when bases are disabled. `isBaseUnit` kept here for that filter.
			if (!GameModeOptionsClass::Instance->Bases
				&& isBaseUnit(candidate)
				&& !ctx.pCollectorOwner->ControlledByCurrentPlayer()) {
				continue;
			}

			Given = candidate;
			break;
		}
	}

	if (Given) {
		if (auto pCreatedUnit = Given->CreateObject(ctx.pCollectorOwner)) {
			if (!pCreatedUnit->Unlimbo(ctx.locSound, DirType::Min)) {
				ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
				return CollectResult::cannot;
			}

			const auto alternativeCell = MapClass::Instance->NearByLocation(
				ctx.pCell->MapCoords, Given->SpeedType, ZoneType::None,
				Given->MovementZone, 0, 1, 1, 0, 0, 0, 1, CellStruct::Empty, false, false);

			if (alternativeCell.IsValid()) {
				if (pCreatedUnit->Unlimbo(CellClass::Cell2Coord(alternativeCell), DirType::Min)) {
					ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
					return CollectResult::cannot;
				}
			}

			GameDelete<true, false>(pCreatedUnit);
			Crate_GiveMoney(ctx, soloCrateMoney);
			return CollectResult::can;
		} else {
			ctx.pType->PlayAllAffects(ctx.loc, ctx.locSound, ctx.isControlledByPlayer);
			Crate_GiveMoney(ctx, soloCrateMoney);
			return CollectResult::can;
		}
	}

	// No valid unit type found at all -> fall back to money (was implicit
	// fallthrough to HealBase in the original switch; treated as Money instead
	// since that matches the GiveMoney fallback used elsewhere).
	Crate_GiveMoney(ctx, soloCrateMoney);
	return CollectResult::can;
}

// Dispatch table: maps Powerup -> handler. Built once on first use.
// To add a new VANILLA crate effect: write Crate_Handle_X in
// FakeCellClass_Crate.cpp, declare it in the header, and add one line here.
// For CUSTOM crate types (not in the Powerup enum), use
// Crate_RegisterCustomHandler() instead — see FakeCellClass_Crate.h.
CollectResult Crate_Dispatch(CrateContext& ctx, int& soloCrateMoney)
{
	switch (ctx.data)
	{
	case Powerup::Money:        Crate_GiveMoney(ctx, soloCrateMoney, false); return CollectResult::can;
	case Powerup::Unit:          return Crate_Handle_Unit(ctx, soloCrateMoney);
	case Powerup::HealBase:       return Crate_Handle_HealBase(ctx);
	case Powerup::Explosion:      return Crate_Handle_Explosion(ctx);
	case Powerup::Napalm:         return Crate_Handle_Napalm(ctx);
	case Powerup::Darkness:       return Crate_Handle_Darkness(ctx);
	case Powerup::Reveal:         return Crate_Handle_Reveal(ctx);
	case Powerup::Armor:          return Crate_Handle_Armor(ctx);
	case Powerup::Speed:          return Crate_Handle_Speed(ctx);
	case Powerup::Firepower:       return Crate_Handle_Firepower(ctx);
	case Powerup::Cloak:          return Crate_Handle_Cloak(ctx);
	case Powerup::ICBM:           return Crate_Handle_ICBM(ctx);
	case Powerup::Veteran:        return Crate_Handle_Veteran(ctx);
	case Powerup::Gas:            return Crate_Handle_Gas(ctx);
	case Powerup::Tiberium:       return Crate_Handle_Tiberium(ctx);
	case Powerup::Squad:          return Crate_Handle_Squad(ctx, soloCrateMoney);
	case Powerup::Invulnerability: return Crate_Handle_Invulnerability(ctx);
	case Powerup::IonStorm:       return Crate_Handle_IonStorm(ctx);
	case Powerup::Pod:            return Crate_Handle_Pod(ctx);
	default:                      return Crate_Handle_Default(ctx);
	}
}

CollectResult FakeCellClass::_CollecCrate(FootClass* pCollector)
{
	if (!pCollector || this->OverlayTypeIndex <= -1)
		return CollectResult::can;

	const auto pOverlay = OverlayTypeClass::Array->Items[this->OverlayTypeIndex];

	if (!pOverlay->Crate)
		return CollectResult::can;

	const auto pCollectorOwner = pCollector->Owner;

	const bool isPassiveSkip =
		SessionClass::Instance->GameMode != GameMode::Campaign && pCollectorOwner->Type->MultiplayPassive;

	if (isPassiveSkip)
		return CollectResult::can;

	// --- Trigger spring ---
	if (pOverlay->CrateTrigger && pCollector->AttachedTag)
	{
		Debug::LogInfo("Springing trigger on crate at {},{}", this->MapCoords.X, this->MapCoords.Y);
		pCollector->AttachedTag->SpringEvent(TriggerEvent::PickupCrate, pCollector, CellStruct::Empty);

		if (!pCollector->IsAlive)
			return CollectResult::cannot;

		ScenarioClass::Instance->PickedUpAnyCrate = true;
	}

	// --- Determine reward ---
	bool force_mcv = false;
	int soloCrateMoney = 0;
	Powerup data = Crate_DetermineRewardType(this, pCollector);

	if (SessionClass::Instance->GameMode != GameMode::Campaign)
	{
		data = Crate_EvaluateMultiplayerReward(data, pCollector, pCollectorOwner, this->LandType, force_mcv);
		HouseExtData::IncremetCrateTracking(pCollectorOwner, data);
	}
	else
	{
		data = Crate_ResolveCampaignReward(this, soloCrateMoney);
	}

	// --- Remove crate / spawn replacement ---
	MapClass::Instance->Remove_Crate(&this->MapCoords);

	if (SessionClass::Instance->GameMode != GameMode::Campaign && GameModeOptionsClass::Instance->Crates)
		MapClass::Instance->Place_Random_Crate();

	// --- Build context and dispatch ---
	CrateContext ctx {};
	ctx.pCell = this;
	ctx.pCollector = pCollector;
	ctx.pCollectorOwner = pCollectorOwner;
	ctx.loc = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128, 128 }) + 200);
	ctx.locSound = CellClass::Cell2Coord(this->MapCoords, this->GetFloorHeight({ 128, 128 }));
	ctx.data = data;
	// `data` carries the resolved CrateTypeClass::Array index (cast through
	// Powerup). This is valid for BOTH vanilla entries (index < Powerups::
	// Effects.size(), matches a real Powerup enumerator) and custom entries
	// (index >= that size, no matching enumerator -> IsVanillaPowerup() == false).
	ctx.pType = CrateTypeClass::Array[(int)data].get();
	ctx.something = ctx.pType->Argument.Get();
	ctx.isControlledByPlayer = pCollectorOwner->ControlledByCurrentPlayer();

	// NOTE: force_mcv was passed into the original Unit handler via outer scope.
	// Crate_Handle_Unit currently hardcodes force_mcv = false (see TODO comment
	// there) — wire `force_mcv` through CrateContext if MCV-forcing crates need
	// to be preserved exactly.
	return Crate_HandleCrate(ctx, soloCrateMoney);
}
#endif

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

	if (!pCrate) {
		Debug::FatalErrorAndExit(__FUNCTION__" Missing CrateType[size %d] Pointer for[%d]!", CrateTypeClass::Array.size() , crateType);
	}

	R->EDX(pCrate->Name.data());
	return 0x475A4B;
}
ASMJIT_PATCH(0x475A1F, RulesClass_Put_CrateType, 0x5)
{
	GET(const char*, crate, ECX);

	const int idx = CrateTypeClass::FindIndexById(crate);

	if (idx < 0) {
		Debug::FatalErrorAndExit(__FUNCTION__" Missing CrateType[size %d] index for[%s]!", CrateTypeClass::Array.size(), crate);
	}

	R->EAX(idx);
	return 0x475A24;
}

ASMJIT_PATCH(0x48DE79, CrateTypeFromName, 0x7)
{
	GET(const char*, readedName, EBX);

	const auto idx = CrateTypeClass::FindIndexById(readedName);

	if (idx >= 0) {
		R->EDI(idx);
		return 0x48DEA2;
	}

	Debug::FatalErrorAndExit(__FUNCTION__" Missing CrateType[size %d] index for[%s]!", CrateTypeClass::Array.size(), readedName);
	return 0x48DE9C;
}