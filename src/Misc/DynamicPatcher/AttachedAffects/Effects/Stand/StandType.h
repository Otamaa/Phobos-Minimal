#pragma once

#include <Utilities/TemplateDef.h>

struct StandType : public EffectType
{
	StandType(const StandType& nAnother) :
		Type { nAnother.Type }
		, Offset { nAnother.Offset.Get() }
		, Direction { nAnother.Direction.Get() }
		, LockDirection { nAnother.LockDirection.Get() }
		, IsOnTurret { nAnother.IsOnTurret.Get() }
		, IsOnWorld { nAnother.IsOnTurret.Get() }
		, DrawLayer { nAnother.DrawLayer.Get() }
		, ZOffset { nAnother.ZOffset.Get() }
		, SameHouse { nAnother.SameHouse.Get() }
		, SameTarget { nAnother.SameTarget.Get() }
		, SameLoseTarget { nAnother.SameLoseTarget.Get() }
		, ForceAttackMaster { nAnother.ForceAttackMaster.Get() }
		, MobileFire { nAnother.MobileFire.Get() }
		, Powered { nAnother.Powered.Get() }
		, Explodes { nAnother.Explodes.Get() }
		, ExplodesWithMaster { nAnother.ExplodesWithMaster.Get() }
		, RemoveAtSinking { nAnother.RemoveAtSinking.Get() }
		, PromoteFormMaster { nAnother.PromoteFormMaster.Get() }
		, ExperienceToMaster { nAnother.ExperienceToMaster.Get() }
		, VirtualUnit { nAnother.VirtualUnit.Get() }
		, SameTilter { nAnother.SameTilter.Get() }
		, IsTrain { nAnother.IsTrain.Get() }
		, CabinHead { nAnother.CabinHead.Get() }
		, CabinGroup { nAnother.CabinGroup.Get() }
	{ }

	StandType() :
		Type { nullptr }
		, Offset { {0,0,0} }
		, Direction { 0 }
		, LockDirection { false }
		, IsOnTurret { false }
		, IsOnWorld { false }
		, DrawLayer { Layer::None }
		, ZOffset { 12 }
		, SameHouse { true }
		, SameTarget { true }
		, SameLoseTarget { false }
		, ForceAttackMaster { false }
		, MobileFire { true }
		, Powered { false }
		, Explodes { false }
		, ExplodesWithMaster { false }
		, RemoveAtSinking { false }
		, PromoteFormMaster { false }
		, ExperienceToMaster { 0.0 }
		, VirtualUnit { false }
		, SameTilter { true }
		, IsTrain { false }
		, CabinHead { false }
		, CabinGroup { -1 }
	{ }

	PhobosFixedString<0x100> Type; // 替身类型
	Valueable<CoordStruct> Offset; // 替身相对位置
	// public Direction Direction; // 相对朝向
	Valueable<int> Direction; // 相对朝向，16分圆，[0-15]
	Valueable<bool> LockDirection; // 强制朝向，不论替身在做什么
	Valueable<bool> IsOnTurret; // 相对炮塔或者身体
	Valueable<bool> IsOnWorld; // 相对世界
	Valueable<Layer> DrawLayer; // 渲染的层
	Valueable<int> ZOffset; // ZAdjust偏移值
	Valueable<bool> Powered; // 是否需要电力支持
	Valueable<bool> SameHouse; // 与使者同所属
	Valueable<bool> SameTarget; // 与使者同个目标
	Valueable<bool> SameLoseTarget; // 使者失去目标时替身也失去
	Valueable<bool> ForceAttackMaster; // 强制选择使者为目标
	Valueable<bool> MobileFire; // 移动攻击
	Valueable<bool> Explodes; // 死亡会爆炸
	Valueable<bool> ExplodesWithMaster; // 使者死亡时强制替身爆炸
	Valueable<bool> RemoveAtSinking; // 沉船时移除
	Valueable<bool> PromoteFormMaster; // 与使者同等级
	Valueable<double> ExperienceToMaster; // 经验给使者
	Valueable<bool> VirtualUnit; // 不可被选择
	Valueable<bool> SameTilter; // 同步倾斜
	Valueable<bool> IsTrain; // 火车类型
	Valueable<bool> CabinHead; // 插入车厢前端
	Valueable<int> CabinGroup; // 车厢分组

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Type)
			.Process(Offset)
			.Process(Direction)
			.Process(LockDirection)
			.Process(IsOnTurret)
			.Process(IsOnWorld)
			.Process(DrawLayer)
			.Process(ZOffset)
			.Process(SameHouse)
			.Process(SameTarget)
			.Process(SameLoseTarget)
			.Process(ForceAttackMaster)
			.Process(MobileFire)
			.Process(Powered)
			.Process(Explodes)
			.Process(ExplodesWithMaster)
			.Process(RemoveAtSinking)
			.Process(PromoteFormMaster)
			.Process(ExperienceToMaster)
			.Process(VirtualUnit)
			.Process(SameTilter)
			.Process(IsTrain)
			.Process(CabinHead)
			.Process(CabinGroup)
			.Success()
			;

	}

};
