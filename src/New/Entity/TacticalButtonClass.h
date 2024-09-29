#pragma once

#include <ControlClass.h>

class TacticalButtonClass : public ControlClass
{
public:
	static inline constexpr int StartID = 2200;

	TacticalButtonClass() = default;
	TacticalButtonClass(unsigned int id, int superIdx, int x, int y, int width, int height);

	virtual ~TacticalButtonClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag fags, DWORD* pKey, KeyModifier modifier);

	constexpr void SetColumn(int column) {
		this->ColumnIndex = column;
	}
	bool LaunchSuper() const;

public:

	bool IsHovering { false };
	int ColumnIndex { -1 };
	int SuperIndex { -1 };
};