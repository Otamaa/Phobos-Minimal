#pragma once
#include "AttachedEffects.h"
#include <ArrayClasses.h>
#include "AttachEffectData.h"

#include <BulletClass.h>
#include "LocationMark.h"
#include "Effects/AttachStatus/AttachStatusType.h"
#include "Effects/AttachStatus/AttachStatusTypeB.h"

struct AttachEffectManager
{
	std::vector<std::unique_ptr<AttachedAffects>> AttachEffects; // 所有有效的AE
	std::map<const char*, CDTimerClass> DisableDelayTimers; // 同名AE失效后再赋予的计时器
	std::vector<LocationMark> LocationMarks;


private:
	CoordStruct lastLocation; // 使者的上一次位置
	int locationMarkDistance; // 多少格记录一个位置
	double totleMileage; // 总里程

	bool renderFlag; //= false Render比Update先执行，在附着对象Render时先调整替身位置，Update就不用调整
public:
	int LocationSpace; // 替身火车的车厢间距

	AttachEffectManager() :
		AttachEffects {}
		, DisableDelayTimers { }
		, LocationMarks { }
		, lastLocation { 0,0,0 }
		, locationMarkDistance { 16 }
		, totleMileage { 0.0 }
		, renderFlag { false }
		, LocationSpace { 512 }
	{ }

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

	int Count()
	{
		return (int)AttachEffects.size();
	}

	CoordStruct GetLastLoc() const
	{
		return lastLocation;
	}

	void SetLocationSpace(int cabinLenght)
	{
		LocationSpace = cabinLenght;

		if (cabinLenght < locationMarkDistance)
		{
			locationMarkDistance = cabinLenght;
		}
	}

	void Attach(AttachEffectData aeData, ObjectClass* pOwner, HouseClass* pHouse, bool attachOnceFlag)
	{
		if (aeData.Types.empty())
			return;

		TechnoClass* pAttacker = nullptr;

		switch (pOwner->WhatAmI())
		{
		case AbstractType::Unit:
		case AbstractType::Aircraft:
		case AbstractType::Building:
		case AbstractType::Infantry:
			pAttacker = (TechnoClass*)pOwner;
			break;
		case AbstractType::Bullet:
			pAttacker = ((BulletClass*)pOwner)->Owner;
			break;

		}

		for (auto const& type : aeData.Types)
		{
			if (!GeneralUtils::IsValidString(type->Name.data()))
				continue;

			Attach(type->Name.data(), pOwner, pHouse, pAttacker, attachOnceFlag);
		}

		if (aeData.CabinLength > 0)
		{
			SetLocationSpace(aeData.CabinLength);
		}
	}

	void Attach(const char* type, ObjectClass* pOwner, HouseClass* pHouse, TechnoClass* pAttacker, bool attachOnceFlag = true)
	{
		if (!GeneralUtils::IsValidString(type))
			return;

		auto aeType = AttachEffectType::FindOrAllocate(type);
		if (!aeType)
			return;

		if (attachOnceFlag && aeType->AttachOnceInTechnoType)
		{
			return;
		}

		Attach(aeType, pOwner, pHouse, pAttacker);
	}

	void Attach(AttachEffectType* aeType, ObjectClass* pOwner, HouseClass* pHouse, TechnoClass* pAttacker)
	{
		if (!GeneralUtils::IsValidString(aeType->Name.data()))
			return;

		if (!aeType->AffectTypes.empty())
		{
			if (!aeType->AffectTypes.Contains(pOwner->get_ID()))
			{
				return;
			}
		}

		if (!aeType->NotAffectTypes.empty())
		{
			if (aeType->NotAffectTypes.Contains(pOwner->get_ID()))
			{
				return;
			}
		}

		CoordStruct location = pOwner->GetCoords();


		bool add = aeType->Cumulative == CumulativeMode::YES;
		if (!add)
		{

			bool isAttackMark = aeType->Cumulative == CumulativeMode::ATTACKER
				&& pAttacker && pAttacker->IsAlive;

			if (!isAttackMark)
			{
				auto const& nTimer = DisableDelayTimers[aeType->Name.data()];
				if (nTimer.InProgress())
				{
					return;
				}
			}

			bool find = false;

			for (int i = Count() - 1; i >= 0; i--)
			{
				auto& temp = AttachEffects[i];
				if (!temp || !temp->Type)
					continue;

				if (aeType->Group < 0)
				{
					if (std::strcmp(temp->Type->Name.c_str(), aeType->Name.c_str()) == 0)
					{
						find = true;
						if (isAttackMark)
						{
							if (temp->Attacker == pAttacker)
							{
								if (temp->Type->ResetDurationOnReapply)
								{
									temp->ResetDuration();
								}
								break;
							}
						}
						else
						{
							if (temp->Type->ResetDurationOnReapply)
							{
								temp->ResetDuration();
							}
						}
					}
				}
				else
				{
					if (temp->IsSameGroup(aeType))
					{
						find = true;
						if (aeType->OverrideSameGroup)
						{
							temp->Disable(location);
							add = true;
							continue;
						}
						else
						{
							temp->MergeDuation(aeType->Duration);
						}
					}
				}
			}
			add = add || !find;
		}

		if (add && aeType->Duration > 0)
		{
			if (auto ae = std::make_unique<AttachedAffects>(aeType))
			{
				int index = FindInsertIndex(ae);
				if (index != -1)
				{
					AttachEffects[index].reset(std::move(ae));
					AttachEffects[index]->Enable(pOwner, pHouse, pAttacker);
				}
			}
		}
	}

	int FindInsertIndex(AttachedAffects*& ae)
	{
		if (ae->GetStand() && ae->GetStand()->TypeData->IsTrain.Get())
		{
			auto& type = ae->Type->StandTypeData;
			int index = -1;

			if (type.CabinHead.Get())
			{
				if (type.CabinGroup.Get() > -1)
				{
					for (int j = Count() - 1; j >= 0; j--)
					{
						auto& temp = AttachEffects[j];
						if (!temp || !temp->Type || !temp->GetStand())
							continue;

						auto& thattype = temp->Type->StandTypeData;
						if (type.CabinGroup == thattype.CabinGroup)
						{
							index = j;
							break;
						}
					}
				}
			}
			else
			{
				index = -1;
				if (type.CabinGroup.Get() > -1)
				{
					for (int j = 0; j < Count(); j++)
					{
						auto& temp = AttachEffects[j];
						if (!temp || !temp->Type || !temp->GetStand())
							continue;

						auto& thattype = temp->Type->StandTypeData;
						if (type.CabinGroup.Get() == thattype.CabinGroup.Get())
						{
							index = j;
							break;
						}
					}
				}
			}
			if (index > -1)
			{
				return index;
			}
		}

		for (size_t n = 0; n < AttachEffects.size(); n++)
		{
			// find suitable index for this
			if (!AttachEffects[n])
				return n;
		}

		return -1;
	}


	bool HasSpace()
	{
		return totleMileage > Count() * LocationSpace;
	}

	bool HasStand()
	{
		for (auto const& ae : AttachEffects)
		{
			if (ae->GetStand() && ae->IsActive())
			{
				return true;
			}
		}
		return false;
	}

	AttachStatusTypeB CountAttachStatusMultiplier()
	{
		AttachStatusTypeB multiplier;
		for (auto const& ae : AttachEffects)
		{
			if (!ae || !ae->Type)
				continue;

			if (ae->GetStatus() && ae->GetStatus()->Active)
			{
				multiplier.FirepowerMultiplier *= ae->GetStatus()->TypeData->FirepowerMultiplier.Get();
				multiplier.ArmorMultiplier *= ae->GetStatus()->TypeData->ArmorMultiplier.Get();
				multiplier.SpeedMultiplier *= ae->GetStatus()->TypeData->SpeedMultiplier.Get();
				multiplier.ROFMultiplier *= ae->GetStatus()->TypeData->ROFMultiplier.Get();
				multiplier.Cloakable |= ae->GetStatus()->TypeData->Cloakable.Get();
				multiplier.ForceDecloak |= ae->GetStatus()->TypeData->ForceDecloak.Get();
			}
		}
		return multiplier;
	}

	void Put(ObjectClass* pOwner, CoordStruct* pCoord, DirStruct faceDir)
	{
		for (auto const& ae : AttachEffects)
		{
			if (!ae || !ae->Type)
				continue;

			if (ae->IsActive())
			{
				ae->OnPut(pOwner, pCoord, faceDir);
			}
		}
	}

	void Remove(ObjectClass* pOwner)
	{
		CoordStruct location = pOwner->GetCoords();
		for (auto const& ae : AttachEffects)
		{
			if (!ae || !ae->Type)
				continue;

			if (ae->Type->DiscardOnEntry)
			{
				ae->Disable(location);
			}
			else
			{
				if (ae->IsActive())
				{
					ae->OnRemove(pOwner);
				}
			}
		}
	}

	void Update(ObjectClass* pOwner, bool isDead);
	void Render2(ObjectClass* pOwner, bool isDead);

	void ReceiveDamage(ObjectClass* pOwner, int* pDamage, int distanceFromEpicenter, WarheadTypeClass* pWH,
			ObjectClass* pAttacker, bool ignoreDefenses, bool preventPassengerEscape, HouseClass* pAttackingHouse)
	{
		for (auto const& ae : AttachEffects)
		{
			if (!ae || !ae->Type)
				continue;

			if (ae->IsActive())
			{
				ae->OnReceiveDamage(pOwner, pDamage, distanceFromEpicenter, pWH, pAttacker, ignoreDefenses, preventPassengerEscape, pAttackingHouse);
			}
		}

	}
	void DestroyAll(ObjectClass* pOwner)
	{
		for (auto const& ae : AttachEffects)
		{
			if (!ae || !ae->Type)
				continue;

			if (ae->IsActive())
			{
				ae->OnDestroy(pOwner);
			}
		}
	}

	void UnInitAll(CoordStruct location)
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			auto ae = AttachEffects[i];
			if (!ae || !ae->Type)
				continue;

			if (ae->IsActive())
			{
				ae->Disable(location);
			}

			//GameDelete(ae);
			AttachEffects.erase(AttachEffects.begin() + i);
		}
		AttachEffects.clear();
	}

	void GuardCommand()
	{
		for (auto const& ae : AttachEffects)
		{
			if (!ae || !ae->Type)
				continue;

			if (ae->IsActive())
			{
				ae->OnGuardCommand();
			}
		}
	}

	void StopCommand()
	{
		for (auto const& ae : AttachEffects)
		{
			if (!ae || !ae->Type)
				continue;

			if (ae->IsActive())
			{
				ae->OnStopCommand();
			}
		}
	}

private:

	CoordStruct MarkLocation(ObjectClass* pOwner);

	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::LogInfo("Processing Element From AttachEffectManager ! ");
		return Stm
			.Process(AttachEffects)
			.Process(DisableDelayTimers)
			.Process(LocationMarks)
			.Process(lastLocation)
			.Process(locationMarkDistance)
			.Process(totleMileage)
			.Process(renderFlag)
			.Process(LocationSpace)
			.Success()
			;
	}
};