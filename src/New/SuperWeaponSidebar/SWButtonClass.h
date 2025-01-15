#pragma once

#include <ControlClass.h>

class SWButtonClass : public ControlClass
{
public:
	SWButtonClass() = default;
	SWButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height);

	~SWButtonClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag fags, DWORD* pKey, KeyModifier modifier) override;

	COMPILETIMEEVAL void SetColumn(int column)
	{
		this->ColumnIndex = column;
	}

	bool LaunchSuper() const;

public:
	static COMPILETIMEEVAL int StartID = 2200;
	static COMPILETIMEEVAL int ToolTip_Align_Y = 27;

	bool IsHovering { false };
	int ColumnIndex { -1 };
	int SuperIndex { -1 };
};