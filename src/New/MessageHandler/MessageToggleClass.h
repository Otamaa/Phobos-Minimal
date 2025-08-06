#pragma once

#include <GadgetClass.h>

class MessageToggleClass : public GadgetClass
{
public:
	static constexpr int ButtonSide = 18;
	static constexpr int ButtonIconWidth = 4;
	static constexpr int ButtonHeight = ButtonIconWidth + 2;

	MessageToggleClass() = default;
	MessageToggleClass(int x, int y, int width, int height);

	~MessageToggleClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override;

	void DrawShape() const;

public:

	bool Hovering { false };
	bool Clicking { false };
};