#pragma once

#include <Utilities/TemplateDef.h>

class StandType
{
public :

	PhobosFixedString<32> Name;
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

	StandType() : Name { }
		, Offset { }
		,Direction { }
		,LockDirection { }
		,IsOnTurret { }
		,IsOnWorld { }
		,DrawLayer { }
		,ZOffset { }
		,SameHouse { }
		,SameTarget { }
		,SameLoseTarget { }
		,ForceAttackMaster { }
		,MobileFire { }
		,Powered { }
		,Explodes { }
		,ExplodesWithMaster { }
		,RemoveAtSinking { }
		,PromoteFormMaster { }
		,ExperienceToMaster { }
		,VirtualUnit { }
		,SameTilter { }
		,IsTrain { }
		,CabinHead { }
		,CabinGroup { }
	{ }

	void Read(INI_EX& nParser, const char* pSection)
	{
		standType = null;
		// Logger.Log("替身类型 {0} 读取INI配置", section);
		string type = null;
		if (reader.ReadNormal(section, "Stand.Type", ref type) && !string.IsNullOrEmpty(type))
		{
			standType = new StandType();

			standType.Type = type.Trim();
			// Logger.Log("替身类型 {0} 名为 {1}", section, type);

			CoordStruct offset = default;
			if (ExHelper.ReadCoordStruct(reader, section, "Stand.Offset", ref offset))
			{
				standType.Offset = offset;
			}

			string dir = "N";
			if (reader.ReadNormal(section, "Stand.Direction", ref dir))
			{
				Direction direction = (Direction)Enum.Parse(typeof(Direction), dir);
				standType.Direction = (int)direction * 2;
			}

			int dirNum = 0;
			if (reader.ReadNormal(section, "Stand.Direction", ref dirNum))
			{
				standType.Direction = dirNum;
			}

			bool lockDirection = true;
			if (reader.ReadNormal(section, "Stand.LockDirection", ref lockDirection))
			{
				standType.LockDirection = lockDirection;
			}

			bool isOnTurret = true;
			if (reader.ReadNormal(section, "Stand.IsOnTurret", ref isOnTurret))
			{
				standType.IsOnTurret = isOnTurret;
			}

			bool isOnWorld = true;
			if (reader.ReadNormal(section, "Stand.IsOnWorld", ref isOnWorld))
			{
				standType.IsOnWorld = isOnWorld;
			}

			string layerStr = "None";
			if (reader.ReadNormal(section, "Stand.DrawLayer", ref layerStr))
			{
				Layer layer = Layer.None;
				string t = layerStr.Substring(0, 1).ToUpper();
				switch (t)
				{
				case "U":
					layer = Layer.Underground;
					break;
				case "S":
					layer = Layer.Surface;
					break;
				case "G":
					layer = Layer.Ground;
					break;
				case "A":
					layer = Layer.Air;
					break;
				case "T":
					layer = Layer.Top;
					break;
				}
				standType.DrawLayer = layer;
			}

			int zOffset = 12;
			if (reader.ReadNormal(section, "Stand.ZOffset", ref zOffset))
			{
				standType.ZOffset = zOffset;
			}

			bool sameHouse = true;
			if (reader.ReadNormal(section, "Stand.SameHouse", ref sameHouse))
			{
				standType.SameHouse = sameHouse;
			}

			bool sameTarget = true;
			if (reader.ReadNormal(section, "Stand.SameTarget", ref sameTarget))
			{
				standType.SameTarget = sameTarget;
			}

			bool sameLoseTarget = true;
			if (reader.ReadNormal(section, "Stand.SameLoseTarget", ref sameLoseTarget))
			{
				standType.SameLoseTarget = sameLoseTarget;
			}

			bool forceAttackMaster = false;
			if (reader.ReadNormal(section, "Stand.ForceAttackMaster", ref forceAttackMaster))
			{
				standType.ForceAttackMaster = forceAttackMaster;
			}

			bool mobileFire = true;
			if (reader.ReadNormal(section, "Stand.MobileFire", ref mobileFire))
			{
				standType.MobileFire = mobileFire;
			}

			bool powered = true;
			if (reader.ReadNormal(section, "Stand.Powered", ref powered))
			{
				standType.Powered = powered;
			}

			bool explodes = false;
			if (reader.ReadNormal(section, "Stand.Explodes", ref explodes))
			{
				standType.Explodes = explodes;
			}

			bool explodesWithMaster = false;
			if (reader.ReadNormal(section, "Stand.ExplodesWithMaster", ref explodesWithMaster))
			{
				standType.ExplodesWithMaster = explodesWithMaster;
			}

			bool removeAtSinking = false;
			if (reader.ReadNormal(section, "Stand.RemoveAtSinking", ref removeAtSinking))
			{
				standType.RemoveAtSinking = removeAtSinking;
			}

			bool promoteFormMaster = false;
			if (reader.ReadNormal(section, "Stand.PromoteFormMaster", ref promoteFormMaster))
			{
				standType.PromoteFormMaster = promoteFormMaster;
			}

			double experienceToMaster = 0.0;
			if (reader.ReadNormal(section, "Stand.ExperienceToMaster", ref experienceToMaster))
			{
				if (experienceToMaster > 1.0)
				{
					experienceToMaster = 1.0;
				}
				else if (experienceToMaster < 0.0)
				{
					experienceToMaster = 0.0;
				}
				standType.ExperienceToMaster = experienceToMaster;
			}

			bool virtualUnit = false;
			if (reader.ReadNormal(section, "Stand.VirtualUnit", ref virtualUnit))
			{
				standType.VirtualUnit = virtualUnit;
			}

			bool sameTilter = false;
			if (reader.ReadNormal(section, "Stand.SameTilter", ref sameTilter))
			{
				standType.SameTilter = sameTilter;
			}

			bool isTrain = false;
			if (reader.ReadNormal(section, "Stand.IsTrain", ref isTrain))
			{
				standType.IsTrain = isTrain;
			}

			bool cabinHead = false;
			if (reader.ReadNormal(section, "Stand.IsCabinHead", ref cabinHead))
			{
				standType.CabinHead = cabinHead;
			}

			int cabinGroup = -1;
			if (reader.ReadNormal(section, "Stand.CabinGroup", ref cabinGroup))
			{
				standType.CabinGroup = cabinGroup;
			}
		}
		return null != standType;
	}


}
