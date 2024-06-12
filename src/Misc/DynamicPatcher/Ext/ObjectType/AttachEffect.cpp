#include "AttachEffect.h"

#include <BuildingClass.h>
#include <MissionClass.h>

#include <Misc/DynamicPatcher/Common/INI/INI.h>

#include <Misc/DynamicPatcher/Ext/Helper/DrawEx.h>
#include <Misc/DynamicPatcher/Ext/Helper/MathEx.h>
#include <Misc/DynamicPatcher/Ext/Helper/Scripts.h>
#include <Misc/DynamicPatcher/Ext/Helper/Status.h>
#include <Misc/DynamicPatcher/Ext/Helper/SurfaceEx.h>
#include <Misc/DynamicPatcher/Ext/Helper/Weapon.h>
#include <Misc/DynamicPatcher/Ext/Common/PrintTextManager.h>

#include <Misc/DynamicPatcher/Ext/EffectType/AttachEffectScript.h>
#include <Misc/DynamicPatcher/Ext/EffectType/Effect/AttackBeaconEffect.h>
#include <Misc/DynamicPatcher/Ext/EffectType/Effect/StandEffect.h>
#include <Misc/DynamicPatcher/Ext/BulletType/BulletStatus.h>
#include <Misc/DynamicPatcher/Ext/TechnoType/TechnoStatus.h>
#include <Misc/DynamicPatcher/Ext/TechnoType/UploadAttachData.h>
#include <Misc/DynamicPatcher/Ext/WeaponType/FeedbackAttachData.h>

#include <Misc/DynamicPatcher/Ext/StateType/State/AntiBulletData.h>
#include <Misc/DynamicPatcher/Ext/StateType/State/DestroyAnimData.h>
#include <Misc/DynamicPatcher/Ext/StateType/State/DestroySelfData.h>
#include <Misc/DynamicPatcher/Ext/StateType/State/GiftBoxData.h>
#include <Misc/DynamicPatcher/Ext/StateType/State/PaintballData.h>
#include <Misc/DynamicPatcher/Ext/StateType/State/TeleportData.h>
#include <Misc/DynamicPatcher/Ext/StateType/State/TransformData.h>

BulletStatus* AttachEffect::GetBulletStatus()
{
	BulletStatus* status = nullptr;
	if (pBullet && _parent)
	{
		status = _parent->GetComponent<BulletStatus>();
	}
	return status;
}

TechnoStatus* AttachEffect::GetTechnoStatus()
{
	TechnoStatus* status = nullptr;
	if (pTechno && _parent)
	{
		status = _parent->GetComponent<TechnoStatus>();
	}
	return status;
}

void AttachEffect::OnDestroySelf()
{
	_ownerIsDead = true;
}

bool AttachEffect::OwnerIsDead()
{
	if (!_ownerIsDead)
	{
		if (pBullet && IsDead(pBullet))
		{
			_ownerIsDead = true;
		}
		else if (pTechno && (IsDead(pTechno) || InSelling())) // 变卖了建筑也算死亡
		{
			_ownerIsDead = true;
		}
	}
	return _ownerIsDead;
}

int AttachEffect::Count()
{
	return _children.size();
}

void AttachEffect::GetMarks(std::vector<std::string>& marks)
{
	ForeachChild([&marks](Component* c) {
		if (c->c_Type & ComponentType::ObjectAEScript) {
			reinterpret_cast<AttachEffectScript*>(c)->GetMarks(marks);
		}
		});
}

std::vector<std::string> AttachEffect::GetMarks()
{
	std::vector<std::string> marks{};
	GetMarks(marks);
	return marks;
}

void AttachEffect::GetAENames(std::vector<std::string>& names)
{
	ForeachChild([&names](Component* c) {
		if (c->c_Type & ComponentType::ObjectAEScript) {
			names.push_back(reinterpret_cast<AttachEffectScript*>(c)->AEData.Name);
		}
		});
}

bool AttachEffect::HasStand()
{
	bool find = false;
	ForeachChild([&find](Component* c) {
		if (c->c_Type & ComponentType::ObjectAEScript)
		{
			find = reinterpret_cast<AttachEffectScript*>(c)->AEData.Stand.Enable;
			if (find)
			{
				reinterpret_cast<AttachEffectScript*>(c)->Break();
			}
		}
		});
	return find;
}

#define GET_STAND_DATA_STATE(cls)\
case EffectDataType::##cls##Data:\
{\
state = status->cls; \
}break; \

void AttachEffect::AEStateToStand(EffectData* pData, int duration, std::string token, bool resetDuration)
{
	ForeachChild([&pData, &duration, &token, &resetDuration](Component* c) {
		if (c->c_Type & ComponentType::ObjectAEScript) {
			auto ae = reinterpret_cast<AttachEffectScript*>(c);
			if (ae->AEData.Stand.Enable && ae->IsAlive())
			{
				if (StandEffect* standEffect = ae->GetComponent<StandEffect>())
				{
					TechnoStatus* status = nullptr;
					if (standEffect->IsAlive() && TryGetStatus<TechnoExt>(standEffect->pStand, status))
					{
						IStateScript* state = nullptr;
						switch (pData->GetType())
						{
						GET_STAND_DATA_STATE(AntiBullet)
						GET_STAND_DATA_STATE(BlackHole)
						GET_STAND_DATA_STATE(DamageReaction)
						GET_STAND_DATA_STATE(Deselect)
						GET_STAND_DATA_STATE(DestroyAnim)
						GET_STAND_DATA_STATE(DestroySelf)
						GET_STAND_DATA_STATE(DisableWeapon)
						GET_STAND_DATA_STATE(Freeze)
						GET_STAND_DATA_STATE(GiftBox)
						GET_STAND_DATA_STATE(NoMoneyNoTalk)
						GET_STAND_DATA_STATE(OverrideWeapon)
						GET_STAND_DATA_STATE(Paintball)
						GET_STAND_DATA_STATE(Pump)
						GET_STAND_DATA_STATE(Scatter)
						GET_STAND_DATA_STATE(Teleport)
						GET_STAND_DATA_STATE(Transform)
						default:
							break;
						}

						if (state)
						{
							if (resetDuration)
							{
								state->ResetDuration(duration, token);
							}
							else
							{
								state->Replace(pData, duration, token, ae);
							}
						}
					}
				}
			}
		}
		});
}

CrateBuffData AttachEffect::CountAttachStatusMultiplier()
{
	CrateBuffData multiplier{};
	// 替身是虚拟炮塔，buff加成取JOJO身上的
	if (pTechno)
	{
		TechnoStatus* status = GetStatus<TechnoExt, TechnoStatus>(pTechno);
		if (status->AmIStand() && status->MyStandData.IsVirtualTurret
			&& !status->MyMasterIsAnim && !IsDead(status->pMyMaster)
			)
		{
			AttachEffect* aem = nullptr;
			if (TryGetAEManager<TechnoExt>(status->pMyMaster, aem))
			{
				return aem->CountAttachStatusMultiplier();
			}
		}
	}
	// 统计AE加成
	ForeachChild([&multiplier](Component* c) {
		if (!(c->c_Type & ComponentType::ObjectAEScript))
			return;

		auto temp = reinterpret_cast<AttachEffectScript*>(c);
		if (temp && temp->IsAlive() && temp->AEData.CrateBuff.Enable)
		{
			multiplier.FirepowerMultiplier *= temp->AEData.CrateBuff.FirepowerMultiplier;
			multiplier.ArmorMultiplier *= temp->AEData.CrateBuff.ArmorMultiplier;
			multiplier.SpeedMultiplier *= temp->AEData.CrateBuff.SpeedMultiplier;
			multiplier.ROFMultiplier *= temp->AEData.CrateBuff.ROFMultiplier;
			multiplier.Cloakable |= temp->AEData.CrateBuff.Cloakable;
			multiplier.ForceDecloak |= temp->AEData.CrateBuff.ForceDecloak;
			multiplier.RangeMultiplier *= temp->AEData.CrateBuff.RangeMultiplier;
			multiplier.RangeCell += temp->AEData.CrateBuff.RangeCell;
		}
		});
	return multiplier;
}

ImmuneData AttachEffect::GetImmuneData()
{
	ImmuneData data{};
	// 统计AE加成
	ForeachChild([&data](Component* c) {

		if (c->c_Type & ComponentType::ObjectAEScript
			&& reinterpret_cast<AttachEffectScript*>(c)->IsAlive()
			&& reinterpret_cast<AttachEffectScript*>(c)->AEData.Immune.Enable
			)
		{
			auto temp = reinterpret_cast<AttachEffectScript*>(c);
			data.Psionics |= temp->AEData.Immune.Psionics;
			data.PsionicWeapons |= temp->AEData.Immune.PsionicWeapons;
			data.Radiation |= temp->AEData.Immune.Radiation;
			data.Poison |= temp->AEData.Immune.Poison;
			data.EMP |= temp->AEData.Immune.EMP;
			data.Parasite |= temp->AEData.Immune.Parasite;
			data.Temporal |= temp->AEData.Immune.Temporal;
			data.IsLocomotor |= temp->AEData.Immune.IsLocomotor;
			std::vector<std::string> antiWH = temp->AEData.Immune.AntiWarheads;
			data.AntiWarheads.assign(antiWH.begin(), antiWH.end());
			std::vector<std::string> acceptWH = temp->AEData.Immune.AcceptWarheads;
			data.AcceptWarheads.assign(acceptWH.begin(), acceptWH.end());
		}
		});
	data.CheckEnable();
	return data;
}

void AttachEffect::SetLocationSpace(int cabinLength)
{
	_locationSpace = cabinLength;
	if (cabinLength < _locationMarkDistance)
	{
		_locationMarkDistance = cabinLength;
	}
}

void AttachEffect::Attach(AttachEffectTypeData* typeData)
{
	if (typeData->Enable)
	{
		Attach(typeData->AttachEffectTypes, {}, _attachOnceFlag, pObject);
	}
	if (typeData->StandTrainCabinLength > 0)
	{
		SetLocationSpace(typeData->StandTrainCabinLength);
	}

}

void AttachEffect::Attach(std::vector<std::string> types, std::vector<double> chances, bool onceCheck,
	ObjectClass* pSource, HouseClass* pSourceHouse,
	CoordStruct warheadLocation, int aeMode, bool fromPassenger)
{
	if (!types.empty())
	{
		int index = 0;
		for (std::string type : types)
		{
			if (Bingo(chances, index))
			{
				Attach(type, onceCheck, pSource, pSourceHouse, warheadLocation, aeMode, fromPassenger);
			}
			index++;
		}
	}
}

void AttachEffect::Attach(std::string type, bool onceCheck,
	ObjectClass* pSource, HouseClass* pSourceHouse,
	CoordStruct warheadLocation, int aeMode, bool fromPassenger)
{
	if (IsNotNone(type))
	{
		AttachEffectData* data = INI::GetConfig<AttachEffectData>(INI::Rules, type.c_str())->Data;
		if (data->Enable)
		{
			if (onceCheck && data->AttachOnceInTechnoType)
			{
				return;
			}
			Attach(*data, pSource, pSourceHouse, warheadLocation, aeMode, fromPassenger);
		}
	}
}

void AttachEffect::Attach(AttachEffectData data,
	ObjectClass* pSource, HouseClass* pSourceHouse,
	CoordStruct warheadLocation, int aeMode, bool fromPassenger)
{
	if (!data.Enable)
	{
		Debug::Log("Warning: Attemp to attach an invalid AE [%s] to [%s]\n", data.Name.c_str(), pObject->GetType()->ID);
		return;
	}
	// 检查是否穿透铁幕
	if (!data.PenetratesIronCurtain && pObject->IsIronCurtained())
	{
		return;
	}
	// 是否在白名单上
	if (!data.CanAffectType(pObject))
	{
		return;
	}
	// 是否需要标记
	if (!IsOnMark(data))
	{
		return;
	}
	// 是否有排斥的AE
	if (HasContradiction(data))
	{
		return;
	}

	TechnoClass* pAttacker = nullptr;
	HouseClass* pAttackingHouse = pSourceHouse;
	// 调整所属
	if (!pSource)
	{
		pSource = pObject;
	}
	TechnoClass* pSourceTechno = nullptr;
	BulletClass* pSourceBullet = nullptr;
	if (CastToTechno(pSource, pSourceTechno))
	{
		pAttacker = pSourceTechno;
		if (!pAttackingHouse)
		{
			pAttackingHouse = pAttacker->Owner;
		}
	}
	else if (CastToBullet(pSource, pSourceBullet))
	{
		pAttacker = pSourceBullet->Owner;
		if (!pAttackingHouse)
		{
			pAttackingHouse = GetSourceHouse(pSourceBullet);
		}
	}
	else
	{
		Debug::Log("Warning: Attach AE [%s] to [%s] form a unknow source!\n", data.Name.c_str(), pObject->GetType()->ID);
		return;
	}
	// 更改所属，如果需要
	if (data.ReceiverOwn && pSource != pObject)
	{
		// 所属设置为接受者
		if (pSourceTechno)
		{
			pAttackingHouse = pSourceTechno->Owner;
		}
		else if (pSourceBullet)
		{
			pAttackingHouse = GetSourceHouse(pSourceBullet);
		}
	}
	// 调整攻击者
	if (!fromPassenger && data.FromTransporter && !IsDead(pAttacker))
	{
		pAttacker = WhoIsShooter(pAttacker);
	}
	// 检查金币
	int needMoney = data.AttachNeedMoney;
	if (needMoney != 0)
	{
		if (pAttackingHouse)
		{
			int money = pAttackingHouse->Available_Money();
			if (needMoney > 0 ? money < needMoney : money > abs(needMoney))
			{
				return;
			}
		}
		else
		{
			Debug::Log("Warning: Attach AE [%s] to [%s] get a unknow source house when check AttachNeedMoney!\n", data.Name.c_str(), pObject->GetType()->ID);
			return;
		}
	}
	needMoney = data.ReceiverNeedMoney;
	if (needMoney != 0)
	{
		if (HouseClass* myHouse = pObject->GetOwningHouse())
		{
			int money = pAttackingHouse->Available_Money();
			if (needMoney > 0 ? money < needMoney : money > abs(needMoney))
			{
				return;
			}
		}
		else
		{
			Debug::Log("Warning: Attach AE [%s] to [%s] get a unknow source house when check ReceiverNeedMoney!\n", data.Name.c_str(), pObject->GetType()->ID);
			return;
		}
	}
	// 检查叠加
	bool add = data.Cumulative == CumulativeMode::YES;
	if (!add)
	{
		// 不同攻击者是否叠加
		bool isAttackMark = fromPassenger || data.Cumulative == CumulativeMode::ATTACKER && pAttacker && pAttacker->IsAlive;
		// 不同所属是否叠加
		bool isHouseMark = data.Cumulative == CumulativeMode::HOUSE;
		// 攻击者标记AE名称相同，但可以来自不同的攻击者，可以叠加，不检查Delay
		// 检查冷却计时器
		if (!isAttackMark && !isHouseMark && IsOnDelay(data))
		{
			return;
		}
		bool find = false;
		CoordStruct location = _location;
		// 检查持续时间，增减Duration
		ForeachChild([&find, &add, &isAttackMark, &isHouseMark, &data, &pAttacker, &pAttackingHouse, &location](Component* c) {

			if (c->c_Type & ComponentType::ObjectAEScript && reinterpret_cast<AttachEffectScript*>(c)->IsAlive())
			{
				auto temp = reinterpret_cast<AttachEffectScript*>(c);
				if (data.Group < 0)
				{
					// 无分组，攻击者标记叠加，或同名重置计时器
					if (temp->AEData.Name == data.Name)
					{
						find = true;
						if (isAttackMark)
						{
							if (temp->pSource == pAttacker)
							{
								// 相同的攻击者，重置持续时间，并跳出循环
								if (temp->AEData.ResetDurationOnReapply)
								{
									temp->ResetDuration();
								}
								c->Break();
								return;
							}
							else
							{
								// 当前条的攻击者不同，设置标记后，继续循环，直到检查完所有的AE
								find = false;
							}
						}
						else if (isHouseMark)
						{
							if (temp->pSourceHouse == pAttackingHouse)
							{
								// 是所属标记，且所属相同，重置持续时间，并跳出循环
								if (temp->AEData.ResetDurationOnReapply)
								{
									temp->ResetDuration();
								}
								c->Break();
								return;
							}
							else
							{
								// 当前条的攻击者不同，设置标记后，继续循环，直到检查完所有的AE
								find = false;
							}
						}
						else
						{
							// 不是标记，重置已存在的AE的持续时间，跳出循环
							if (temp->AEData.ResetDurationOnReapply)
							{
								temp->ResetDuration();
							}
							c->Break();
							return;
						}
					}
				}
				else
				{
					// 有分组，替换或者调整持续时间
					if (temp->IsSameGroup(data))
					{
						// 找到了同组
						find = true;
						if (data.OverrideSameGroup)
						{
							// 与自己不同名的，替换
							if (temp->AEData.Name != data.Name)
							{
								// 执行替换操作，关闭所有的同组AE
								temp->TimeToDie();
								add = true;
							}
							else if (temp->AEData.ResetDurationOnReapply)
							{
								temp->ResetDuration();
							}
							// 继续循环直至全部关闭
						}
						else
						{
							// 调整持续时间
							temp->MergeDuration(data.Duration);
							// 继续循环直至全部调整完
						}
					}
				}
			}
			});
		// 没找到同类或同组，可以添加新的实例
		add = add || !find;
	}
	// 可以添加AE，开始执行添加动作
	if (add && data.GetDuration() != 0 && StackNotFull(data))
	{
		int index = FindInsertIndex(data);
		Component* c = AddComponent(AttachEffectScript::ScriptName, index); // 插队

		if (c)
		{
			AddStackCount(data); // 叠层计数
			// 初始化AE
			auto ae = static_cast<AttachEffectScript*>(c);
			ae->AEData = data;
			// 激活AE
			ae->EnsureAwaked();
			ae->Start(pAttacker, pAttackingHouse, warheadLocation, aeMode, fromPassenger);
		}

	}
}

void AttachEffect::FeedbackAttach(WeaponTypeClass* pWeapon)
{
	// Feedback
	if (pWeapon && !IsDeadOrInvisible(pObject))
	{
		FeedbackAttachData* typeData = INI::GetConfig<FeedbackAttachData>(INI::Rules, pWeapon->ID)->Data;
		if (typeData->Enable)
		{
			bool inTransporter = pTechno && pTechno->Transporter != nullptr;
			TechnoClass* pTransporter = nullptr;
			AttachEffect* pTransporterAEM = nullptr;
			if (inTransporter)
			{
				pTransporter = WhoIsShooter(pTechno);
				pTransporterAEM = GetAEManager<TechnoExt>(pTransporter);
			}
			std::map<int, FeedbackAttachEntity> aeTypes = typeData->Datas;
			for (auto it = aeTypes.begin(); it != aeTypes.end(); it++)
			{
				FeedbackAttachEntity data = it->second;
				if (data.Enable)
				{
					// 检查所属是否平民
					if (pTechno && data.DeactiveWhenCivilian && IsCivilian(pTechno->Owner))
					{
						continue;
					}
					if (pTechno)
					{
						if (data.AffectTechno)
						{
							TechnoClass* tempTechno = pTechno;
							AttachEffect* tempAEM = this;
							// 载具内且能赋予载具，则赋予，不能则跳过
							if (inTransporter)
							{
								if (!pTransporterAEM)
								{
									continue;
								}
								tempTechno = pTransporter;
								tempAEM = pTransporterAEM;
							}
							// 检查是否可以影响并赋予AE
							if (data.CanAffectType(tempTechno)
								&& (data.AffectInAir || !tempTechno->IsInAir())
								// && (e.AffectStand || !AmIStand(tempTechno))
								&& tempAEM->IsOnMark(data)
								)
							{
								tempAEM->Attach(data.AttachEffects, data.AttachChances);
							}
						}
					}
					else if (data.AffectBullet && pBullet)
					{
						if (data.CanAffectType(pBullet) && IsOnMark(data))
						{
							Attach(data.AttachEffects, data.AttachChances);
						}
					}
				}
			}
		}
	}
}

void AttachEffect::AttachUploadAE()
{
	// 读取所有的乘客，获取Id清单，并依据乘客的设置，为载具添加AE
	PassengerIds.clear();
	// 坦克碉堡
	if (IsBuilding())
	{
		CheckAndAttachUploadAE(pTechno->BunkerLinkedItem);
	}
	// 乘客舱
	if (pTechno->Passengers.NumPassengers > 0)
	{
		// 有乘客
		ObjectClass* pPassenger = pTechno->Passengers.FirstPassenger;
		do
		{
			CheckAndAttachUploadAE(generic_cast<TechnoClass*>(pPassenger));
		} while ((pPassenger = pPassenger->NextObject) != nullptr);
	}
}

void AttachEffect::CheckAndAttachUploadAE(TechnoClass* pPassenger)
{
	if (!IsDead(pPassenger))
	{
		// 查找乘客身上的AEMode设置
		AttachEffect* aeManager = nullptr;
		if (TryGetAEManager<TechnoExt>(pPassenger, aeManager))
		{
			int aeMode = aeManager->GetTypeData()->AEMode;
			if (aeMode >= 0)
			{
				PassengerIds.push_back(aeMode);
			}
		}
		// 查找乘客身上的AE设置，赋予载具
		UploadAttachData* uploadData = INI::GetConfig<UploadAttachData>(INI::Rules, pPassenger->GetType()->ID)->Data;
		if (uploadData->Enable)
		{
			std::map<int, UploadAttachEntity> uploadDatas = uploadData->Datas;
			for (auto it = uploadDatas.begin(); it != uploadDatas.end(); it++)
			{
				UploadAttachEntity e = it->second;
				if (e.Enable
					&& e.CanAffectType(pTechno)
					&& (e.AffectInAir || !pTechno->IsInAir())
					// && (e.AffectStand || !AmIStand(pTechno))
					&& this->IsOnMark(e)
					)
				{
					Attach(e.AttachEffects, {}, false, pPassenger, nullptr, CoordStruct::Empty, -1, e.SourceIsPassenger);
				}
			}
		}
	}
}

void AttachEffect::AttachGroupAE()
{
	if (GetGroupData()->Enable)
	{
		std::map<int, AttachEffectTypeData> groupData = GetGroupData()->Datas;
		for (auto it = groupData.begin(); it != groupData.end(); it++)
		{
			AttachEffectTypeData aeType = it->second;
			if (aeType.AttachByPassenger)
			{
				// 该组AE需要乘客进行激活
				int aeMode = aeType.AEModeIndex;
				auto ite = std::find(PassengerIds.begin(), PassengerIds.end(), aeMode);
				if (ite != PassengerIds.end())
				{
					// 乘客中有这个ID
					Attach(aeType.AttachEffectTypes, {}, false, pObject, nullptr, CoordStruct::Empty, aeMode, false);
				}
			}
			else
			{
				// 该组AE不需要乘客激活，直接赋予
				Attach(&aeType);
			}
		}
	}
}

#define ATTACH_EFFECT_ON_TECHNOTYPE(EFFECT_NAME) \
		EFFECT_NAME ## Data* _ ## EFFECT_NAME ## Data = INI::GetConfig<EFFECT_NAME ## Data>(INI::Rules, section.c_str())->Data; \
		if (_ ## EFFECT_NAME ## Data->Enable) \
		{ \
			AttachEffectData aeData; \
			aeData.Enable = true; \
			aeData.Name = section + #EFFECT_NAME; \
			aeData. ## EFFECT_NAME = *_ ## EFFECT_NAME ## Data; \
			Attach(aeData); \
		} \

void AttachEffect::AttachStateEffect()
{
	//TODO Attach Effect on TechnoType
	std::string section = pObject->GetType()->ID;
	if (IsNotNone(section))
	{
		ATTACH_EFFECT_ON_TECHNOTYPE(AttackBeacon);
		ATTACH_EFFECT_ON_TECHNOTYPE(DamageSelf);
		ATTACH_EFFECT_ON_TECHNOTYPE(DestroyAnim);
		ATTACH_EFFECT_ON_TECHNOTYPE(ExtraFire);
		ATTACH_EFFECT_ON_TECHNOTYPE(FireSuper);
		ATTACH_EFFECT_ON_TECHNOTYPE(NoMoneyNoTalk);
	}
}

void AttachEffect::DetachByName(std::vector<std::string> aeTypes, bool skipNext)
{
	ForeachChild([&aeTypes, &skipNext](Component* c) {

		// 通过名字关闭掉AE
		if (c->c_Type & ComponentType::ObjectAEScript && std::find(aeTypes.begin(), aeTypes.end(),
			reinterpret_cast<AttachEffectScript*>(c)->AEData.Name) != aeTypes.end())
		{
			auto ae = reinterpret_cast<AttachEffectScript*>(c);
			ae->SkipNext = skipNext;
			ae->TimeToDie();
		}
		});
}

void AttachEffect::DetachByName(std::map<std::string, int> aeTypes, bool skipNext)
{
	std::vector<std::string> names;
	std::map<std::string, int> levels;
	std::map<std::string, int> counts;
	for (auto ae : aeTypes)
	{
		std::string name = ae.first;
		int level = ae.second;
		names.push_back(name);
		levels[name] = level;
		counts[name] = 0;
	}
	if (!names.empty())
	{
		ForeachChild([&](Component* c) {
			if (!(c->c_Type & ComponentType::ObjectAEScript))
				return;

			auto ae = reinterpret_cast<AttachEffectScript*>(c);
			std::string name = ae->AEData.Name;
			// 通过名字关闭掉AE
			if (std::find(names.begin(), names.end(), name) != names.end())
			{
				int l = levels[name];
				if (l < 0 || counts[name] < l)
				{
					ae->SkipNext = skipNext;
					ae->TimeToDie();
					counts[name]++;
				}
			}
		});
	}
}

void AttachEffect::DetachByMarks(std::vector<std::string> marks, bool skipNext)
{
	ForeachChild([&marks, &skipNext](Component* c) {
		if (!(c->c_Type & ComponentType::ObjectAEScript))
			return;

		auto ae = reinterpret_cast<AttachEffectScript*>(c);
		// 通过标记关闭掉AE
		if (ae->AEData.Mark.Enable && CheckOnMarks(marks, ae->AEData.Mark.Names))
		{
			ae->SkipNext = skipNext;
			ae->TimeToDie();
		}
		});
}

void AttachEffect::DetachByToken(std::string token, bool skipNext)
{
	if (!token.empty())
	{
		ForeachChild([&token, &skipNext](Component* c) {
			if (!(c->c_Type & ComponentType::ObjectAEScript))
				return;

			auto ae = reinterpret_cast<AttachEffectScript*>(c);
			// 通过Token关闭掉AE
			if (ae->Token == token)
			{
				ae->SkipNext = skipNext;
				ae->TimeToDie();
			}
			});
	}
}

void AttachEffect::DetachWhenTransform()
{
	ForeachChild([](Component* c) {
		if (!(c->c_Type & ComponentType::ObjectAEScript))
			return;

		auto ae = reinterpret_cast<AttachEffectScript*>(c);
		// 通过标记关闭掉AE
		if (ae->AEData.DiscardOnTransform && !ae->AEData.Transform.Enable)
		{
			ae->TimeToDie();
		}
		});
}

void AttachEffect::CheckDurationAndDisable(bool silence)
{
	CoordStruct location = _location;
	ForeachChild([&](Component* c) {
		if (!(c->c_Type & ComponentType::ObjectAEScript))
			return;

		auto ae = reinterpret_cast<AttachEffectScript*>(c);
		{
			// 执行IsAlive时，检查AE的生命状态，失效的AE会在这里被标记为Deactivate
			if (!ae->IsAlive())
			{
				AttachEffectData data = ae->AEData;
				// 结束AE
				ae->End(location);
				if (!silence)
				{
					// 加入冷却计时器
					StartDelay(data);
					if (!ae->SkipNext)
					{
						// 添加NextAE
						std::string nextAE = data.Next;
						if (IsNotNone(nextAE) && !IsDeadOrInvisible(pObject))
						{
							Attach(nextAE, false, ae->pSource, ae->pSourceHouse);
						}
					}
				}
				ReduceStackCount(data);
				// Deactivate的组件不会再执行Foreach事件，标记为失效，以便父组件将其删除
				ae->Disable();
			}
		}
		});
}

AttachEffectTypeData* AttachEffect::GetTypeData()
{
	if (!_typeData)
	{
		_typeData = INI::GetConfig<AttachEffectTypeData>(INI::Rules, pObject->GetType()->ID)->Data;
	}
	return _typeData;
}

AttachEffectGroupData* AttachEffect::GetGroupData()
{
	if (!_groupData)
	{
		_groupData = INI::GetConfig<AttachEffectGroupData>(INI::Rules, pObject->GetType()->ID)->Data;
	}
	return _groupData;
}

bool AttachEffect::IsOnMark(FilterData data)
{
	if (data.HasMarks())
	{
		std::vector<std::string> marks;
		GetMarks(marks);
		return data.OnMark(marks);
	}
	return true;
}

bool AttachEffect::HasContradiction(AttachEffectData data)
{
	std::vector<std::string> names;
	GetAENames(names);
	return data.HasContradiction(names);
}

bool AttachEffect::IsOnDelay(AttachEffectData data)
{
	std::string name = data.Name;
	auto it = DisableDelayTimers.find(name);
	if (it != DisableDelayTimers.end())
	{
		return it->second.InProgress();
	}
	return false;
}

void AttachEffect::StartDelay(AttachEffectData data)
{
	std::string name = data.Name;
	auto it = DisableDelayTimers.find(name);
	if (it == DisableDelayTimers.end() || it->second.Expired())
	{
		int delay = GetRandomValue(data.RandomDelay, data.Delay);
		if (delay > 0)
		{
			CDTimerClass timer{ delay };
			DisableDelayTimers[name] = timer;
		}
	}
}

void AttachEffect::AddStackCount(AttachEffectData data)
{
	std::string name = data.Name;
	auto it = AEStacks.find(name);
	if (it != AEStacks.end())
	{
		it->second++;
	}
	else
	{
		AEStacks[name] = 1;
	}
}

void AttachEffect::ReduceStackCount(AttachEffectData data)
{
	std::string name = data.Name;
	auto it = AEStacks.find(name);
	if (it != AEStacks.end())
	{
		it->second--;
		if (it->second < 1)
		{
			AEStacks.erase(it);
		}
	}
}

bool AttachEffect::StackNotFull(AttachEffectData data)
{
	if (data.MaxStack > 0)
	{
		std::string name = data.Name;
		auto it = AEStacks.find(name);
		if (it != AEStacks.end())
		{
			return it->second < data.MaxStack;
		}
	}
	return true;
}

CoordStruct AttachEffect::StackOffset(AttachEffectData aeData, OffsetData offsetData,
	std::map<std::string, CoordStruct>& offsetMarks,
	std::map<int, CoordStruct>& groupMarks,
	std::map<int, CoordStruct>& groupFirstMarks)
{
	CoordStruct offset = offsetData.Offset;
	if (offsetData.StackGroup > -1)
	{
		// 分组堆叠
		int stackGroup = offsetData.StackGroup;
		auto it = groupMarks.find(stackGroup);
		if (it != groupMarks.end())
		{
			// 有记录，往上堆叠
			CoordStruct offsetMark = it->second;
			offset = offsetMark + offsetData.StackOffset;
			it->second = offset;
		}
		else
		{
			// 没有记录，取最后一个组的初始偏移位置，加上组偏移
			if (!groupFirstMarks.empty())
			{
				auto ite = groupFirstMarks.end();
				ite--;
				offset = ite->second + offsetData.StackGroupOffset;
			}
			// 创建新的分组
			groupMarks[stackGroup] = offset;
			groupFirstMarks[stackGroup] = offset;
		}
	}
	else if (!offsetData.StackOffset.IsEmpty())
	{
		// 无分组堆叠
		std::string aeName = aeData.Name;
		auto it = offsetMarks.find(aeName);
		if (it != offsetMarks.end())
		{
			// 需要进行偏移
			CoordStruct offsetMark = it->second;
			offset = offsetMark + offsetData.StackOffset;
			it->second = offset;
		}
		else
		{
			offsetMarks[aeName] = offset;
		}

	}
	return offset;
}

int AttachEffect::FindInsertIndex(AttachEffectData data)
{
	int index = -1;
	int size = -1;
	if (data.Stand.Enable && data.Stand.IsTrain && (size = _children.size()) > 0)
	{
		// Head or Trail
		if (data.Stand.CabinHead)
		{
			// check group
			if (data.Stand.CabinGroup > -1)
			{
				// Find the first same group cabin but reverse
				for (auto it = _children.rbegin(); it != _children.rend(); it++)
				{
					if (!((*it)->c_Type & ComponentType::ObjectAEScript))
						continue;

					AttachEffectScript* ae = reinterpret_cast<AttachEffectScript*>(*it);
					if (ae && ae->IsAlive() && ae->AEData.Stand.Enable && ae->AEData.Stand.IsTrain)
					{
						if (ae->AEData.Stand.CabinGroup == data.Stand.CabinGroup)
						{
							index = size - 1;
							break;
						}
					}
					size--;
				}

			}
		}
		else
		{
			index = 0;
			// check group
			if (data.Stand.CabinGroup > -1)
			{
				// Find the first same group cabin
				int i = 0;
				for (Component*& c : _children)
				{
					if (!(c->c_Type & ComponentType::ObjectAEScript))
						continue;

					auto ae = reinterpret_cast<AttachEffectScript*>(c);
					if (ae->IsAlive() && ae->AEData.Stand.Enable && ae->AEData.Stand.IsTrain)
					{
						if (ae->AEData.Stand.CabinGroup == data.Stand.CabinGroup)
						{
							index = i;
							break;
						}
					}
					i++;
				}
			}
		}
	}
	return index;
}

bool AttachEffect::UpdateTrainStandLocation(AttachEffectScript* ae, int& markIndex)
{
	// 查找可以用的记录点
	double length = 0;
	LocationMark preMark;
	for (std::size_t j = markIndex; j < _locationMarks.size(); j++)
	{
		markIndex = j;
		LocationMark mark = _locationMarks[j];
		if (preMark.IsEmpty())
		{
			preMark = mark;
			continue;
		}
		length += mark.Location.DistanceFrom(preMark.Location);
		preMark = mark;
		if (length >= _locationSpace)
		{
			break;
		}
	}

	if (!preMark.IsEmpty())
	{
		ae->UpdateStandLocation(preMark);
		return true;
	}
	return false;
}

CoordStruct AttachEffect::MarkLocation()
{
	CoordStruct location = pObject->GetCoords();
	if (_lastLocation.IsEmpty())
	{
		_lastLocation = location;
	}
	else
	{
		double mileage = location.DistanceFrom(_lastLocation);
		if (!isnan(mileage) && mileage > _locationMarkDistance)
		{
			_lastLocation = location;
			double tempMileage = _totalMileage + mileage;
			// 记录下当前的位置
			OffsetData offset{};
			LocationMark mark = GetRelativeLocation(pObject, offset);
			// 插入队头
			_locationMarks.insert(_locationMarks.begin(), mark);
			// 检查容量(当前AE数量+1)，弹出队尾
			if (tempMileage > (Count() + 1) * _locationSpace)
			{
				_locationMarks.pop_back();
			}
			else
			{
				_totalMileage = tempMileage;
			}
		}
	}
	return location;
}

void AttachEffect::ClearLocationMarks()
{
	_locationMarks.clear();
	_lastLocation = CoordStruct::Empty;
	_totalMileage = 0;
}

void AttachEffect::OnGScreenRender(EventSystem* sender, Event e, void* args)
{
	if (!pObject)
	{
		return;
	}
	CoordStruct location = pObject->GetCoords();
	if (args)
	{
		// EndRender
		ForeachChild([&location](Component* c) {
			if (!(c->c_Type & ComponentType::ObjectAEScript))
				return;

			reinterpret_cast<AttachEffectScript*>(c)->OnGScreenRenderEnd(location);
		});
	}
	else
	{
		// BeginRender
		// 替身的定位偏移
		std::map<std::string, CoordStruct> standMarks{};
		std::map<int, CoordStruct> standGroupMarks{};
		std::map<int, CoordStruct> standGroupFirstMarks{};
		// 动画的定位偏移
		std::map<std::string, CoordStruct> animMarks{};
		std::map<int, CoordStruct> animGroupMarks{};
		std::map<int, CoordStruct> animGroupFirstMarks{};

		// 火车的位置索引
		int markIndex = 0;
		ForeachChild([&](Component* c) {
			if (!(c->c_Type & ComponentType::ObjectAEScript))
				return;

			auto ae = reinterpret_cast<AttachEffectScript*>(c);
			{
				if (ae->IsAlive())
				{
					AttachEffectData aeData = ae->AEData;
					// 调整替身的位置
					if (aeData.Stand.Enable)
					{
						// 调整火车替身的位置
						if (!aeData.Stand.IsTrain || !UpdateTrainStandLocation(ae, markIndex))
						{
							// 堆叠偏移
							OffsetData offsetData = aeData.Stand.Offset;
							CoordStruct standOffset = this->StackOffset(aeData, offsetData, standMarks, standGroupMarks, standGroupFirstMarks);
							LocationMark locationMark = GetRelativeLocation(pObject, offsetData, standOffset);
							ae->UpdateStandLocation(locationMark);
						}
					}
					// 调整动画的位置
					if (aeData.Animation.Enable && aeData.Animation.IdleAnim.Enable)
					{
						OffsetData offsetData = aeData.Animation.IdleAnim.Offset;
						CoordStruct animOffset = this->StackOffset(aeData, offsetData, animMarks, animGroupMarks, animGroupFirstMarks);
						ae->UpdateAnimOffset(animOffset);
					}
					ae->OnGScreenRender(location);
				}
			}
			});
	}
}

void AttachEffect::OnUpdate()
{
	// 移除失效的AE，附加Next的AE
	CheckDurationAndDisable();

	// 添加Section上记录的AE
	if (!_ownerIsDead)
	{
		_location = MarkLocation();
		// 检查电力
		if (!IsBullet())
		{
			PowerOff = pTechno->Owner->HasLowPower();
			if (!PowerOff && IsBuilding())
			{
				// 关闭当前建筑电源
				PowerOff = !static_cast<BuildingClass*>(pTechno)->HasPower;
			}
		}

		// 添加section自带AE
		Attach(GetTypeData());
		// 检查乘客并附加乘客带来的AE
		if (pTechno)
		{
			// 赋予乘客带来的AE，由于乘客不会在非OpenTopped的载具内执行update事件，因此由乘客向载具赋予AE的任务也由载具执行
			AttachUploadAE();
		}
		// 赋予自带的多组AE
		AttachGroupAE();

		this->_attachOnceFlag = true;
	}
}

void AttachEffect::OnWarpUpdate()
{
	// 移除失效的AE，附加Next的AE
	CheckDurationAndDisable();
	if (!_ownerIsDead)
	{
		_location = MarkLocation();
	}
}

void AttachEffect::OnPut(CoordStruct* pCoord, DirType dirType)
{
	_location = *pCoord;
	// 部分AE以状态方式写在TechnoType标签里，在初始化时，自动附加一个AE
	if (!_attachStateEffectFlag)
	{
		_attachStateEffectFlag = true;

		AttachStateEffect();
	}
}

void AttachEffect::OnRemove()
{
	// 从地图移除时关闭AE
	CoordStruct location = pObject->GetCoords();
	_location = location;
	ClearLocationMarks();
	ForeachChild([&location](Component* c) {
		if (!(c->c_Type & ComponentType::ObjectAEScript))
			return;

		auto ae = reinterpret_cast<AttachEffectScript*>(c);
		{
			if (ae->AEData.DiscardOnEntry)
			{
				ae->End(location);
			}
		}
		});
}

void AttachEffect::CanFire(AbstractClass* pTarget, WeaponTypeClass* pWeapon, bool& ceaseFire)
{
	if (pTarget && pWeapon && pWeapon->Warhead)
	{
		// 如果目标免疫超时空和磁电，不能攻击
		TechnoClass* pTargetTechno = nullptr;
		AttachEffect* aem = nullptr;
		if (CastToTechno(pTarget, pTargetTechno) && TryGetAEManager<TechnoExt>(pTargetTechno, aem))
		{
			ImmuneData data = aem->GetImmuneData();
			if (data.Enable)
			{
				// 免疫超时空和磁电
				if ((pWeapon->Warhead->Temporal && data.Temporal)
					|| (pWeapon->Warhead->IsLocomotor && data.IsLocomotor))
				{
					ceaseFire = true;
					Break();
				}
			}
		}
	}
}

void AttachEffect::OnFire(AbstractClass* pTarget, int weaponIdx)
{
	WeaponStruct* pWeapon = pTechno->GetWeapon(weaponIdx);
	WeaponTypeClass* pWeaponType = nullptr;
	if (pWeapon && (pWeaponType = pWeapon->WeaponType) != nullptr)
	{
		FeedbackAttach(pWeaponType);
	}
}

void AttachEffect::OnReceiveDamageDestroy()
{
	_ownerIsDead = true;
};

void AttachEffect::OnDetonate(CoordStruct* pCoords, bool& skip)
{
	_ownerIsDead = true;
	_location = *pCoords;
}

void AttachEffect::OnUnInit()
{
	_ownerIsDead = true;
	CoordStruct location = _location;
	ForeachChild([&location](Component* c) {
		if (!(c->c_Type & ComponentType::ObjectAEScript))
			return;

		auto ae = reinterpret_cast<AttachEffectScript*>(c);
		{
			ae->TimeToDie();
			ae->End(location);
		}
		});
}
