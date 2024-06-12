#pragma once

#include <GeneralDefinitions.h>

#include <Utilities/Debug.h>

#include <Misc/DynamicPatcher/Common/Components/ScriptComponent.h>

#include <Misc/DynamicPatcher/Ext/Helper/Weapon.h>

#include "TrajectoryData.h"

/// @brief 动态载入组件
class KratosStraightTrajectory : public BulletScript
{
public:
	/// @brief 直线导弹的状态数据
	struct StraightBullet
	{
	public:
		CoordStruct sourcePos;
		CoordStruct targetPos;
		VelocityClass Velocity;

		void ResetVelocity(BulletClass* pBullet)
		{
			this->Velocity = RecalculateBulletVelocity(pBullet, sourcePos, targetPos);
		}
	};

	BULLET_SCRIPT(KratosStraightTrajectory);

	void ResetTarget(AbstractClass* pNewTarget, CoordStruct targetPos);

	void ResetStraightMissileVelocity();

	virtual void Clean() override
	{
		BulletScript::Clean();

		_straightBullet = {};
		_resetTargetFlag = false;
	}

	virtual void Awake() override;

	virtual void OnPut(CoordStruct* pCoord, DirType dirType) override;

	virtual void OnUpdate() override;

#pragma region save/load
	template <typename T>
	inline bool Serialize(T& stream)
	{
		return stream
			.Process(this->_straightBullet)
			.Process(this->_resetTargetFlag)
			.Success();
	};

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange);
	virtual bool Save(PhobosStreamWriter& stream) const;
#pragma endregion
private:
	void Setup();

	// 记录直线导弹的位置和速度
	StraightBullet _straightBullet{};
	bool _resetTargetFlag = false;

};
