#pragma once

#include <string>
#include <vector>

#include <Utilities/Debug.h>

#include <Misc/DynamicPatcher/Common/Components/ScriptComponent.h>

#include <Misc/DynamicPatcher/Ext/TrailType/Trail.h>


class BulletTrail : public BulletScript
{
public:
	BULLET_SCRIPT(BulletTrail);

	void SetupTrails();

	virtual void Clean() override
	{
		BulletScript::Clean();

		_setupFlag = false;
		_trails.clear();
	}

	virtual void OnUpdate() override;

	virtual void OnUpdateEnd() override;

private:
	bool _setupFlag = false;

	std::vector<Trail> _trails{};
};
