#pragma once

#include <string>
#include <vector>

#include <Kamikaze.h>

#include <Utilities/Debug.h>

#include <Misc/Kratos/Common/Components/ScriptComponent.h>

#include "MissileHomingData.h"

/// @brief 动态载入组件
class MissileHoming : public TechnoScript
{
public:

	TECHNO_SCRIPT(MissileHoming);

	void Setup();

	// Kamikaze导弹跟踪，重设目标
	bool KamikazeUpdateTarget(Kamikaze::KamikazeControl* pKamikazeControl);

	void OnTechnoDelete(EventSystem* sender, Event e, void* args)
	{
		if (args == HomingTarget)
		{
			HomingTarget = nullptr;
		}
	}

	virtual void Clean() override
	{
		TechnoScript::Clean();

		IsHoming = false;
		HomingTarget = nullptr;
		HomingTargetLocation = CoordStruct::Empty;

		_homingData = nullptr;

		_initHomingFlag = false;
	}

	virtual void Awake() override;

	virtual void OnUpdate() override;

	// 子机导弹跟踪，标记可由Hook强制开启
	bool IsHoming = false;
	// 跟踪的目标，可以是Techno或者Cell，由SpawnManagerClass_Update_Add_Missile_Target写入
	AbstractClass* HomingTarget = nullptr;
	CoordStruct HomingTargetLocation = CoordStruct::Empty;

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->IsHoming)
			.Process(this->HomingTarget)
			.Process(this->HomingTargetLocation)
			.Process(this->_initHomingFlag)
			.Success();
	};
	virtual bool Load(PhobosStreamReader& stream, bool registerForChange) override
	{
		Component::Load(stream, registerForChange);
		return this->Serialize(stream);
	}
	virtual bool Save(PhobosStreamWriter& stream) const override
	{
		Component::Save(stream);
		return const_cast<MissileHoming*>(this)->Serialize(stream);
	}
#pragma endregion

private:
	MissileHomingData* _homingData = nullptr;
	MissileHomingData* GetHomingData();

	bool _initHomingFlag = false;

};
