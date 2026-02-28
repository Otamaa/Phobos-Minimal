#pragma once

#include <GeneralDefinitions.h>
#include <HouseClass.h>

#include "../StateScript.h"
#include "PaintballData.h"

#include <Misc/Kratos/Ext/Common/PaintballSyncManager.h>
#include <Misc/Kratos/Ext/TechnoType/DamageText.h>

class PaintballState : public StateScript<PaintballData>
{
public:
	STATE_SCRIPT(Paintball);

	void RGBIsPower();

	bool NeedPaint(bool& changeColor, bool& changeBright);

	void SyncPaintball();

	virtual void Clean() override
	{
		// 重要：在Clean之前解除同步注册
		// 因为Clean()会改变InstanceId，从而改变thisName
		PaintballSyncManager::Unregister(this);

		StateScript<PaintballData>::Clean();

		_rgbMode = false;
		_rgbIdx = 0;
		_rgbTimer = {};
	}

	virtual void OnStart() override
	{
		SyncPaintball();
	};

	virtual void OnEnd() override
	{
		SyncPaintball();
	};


	virtual void OnInitState(bool replace) override;

	virtual void OnUpdate() override;

	PaintballState& operator=(const PaintballState& other)
	{
		if (this != &other)
		{
			StateScript<PaintballData>::operator=(other);
			_rgbMode = other._rgbMode;
			_rgbIdx = other._rgbIdx;
			_rgbTimer = other._rgbTimer;
		}
		return *this;
	}

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->_rgbMode)
			.Process(this->_rgbIdx)
			.Process(this->_rgbTimer)
			.Success();
	};

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange)
	{
		StateScript<PaintballData>::Load(stream, registerForChange);
		return this->Serialize(stream);
	}
	virtual bool Save(PhobosStreamWriter& stream) const
	{
		StateScript<PaintballData>::Save(stream);
		return const_cast<PaintballState*>(this)->Serialize(stream);
	}
#pragma endregion
private:
	bool _rgbMode = false;
	int _rgbIdx = 0;
	CDTimerClass _rgbTimer{};
};
