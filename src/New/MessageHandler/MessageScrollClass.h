#pragma once
#include <GadgetClass.h>

class MessageScrollClass : public GadgetClass
{
public:
	MessageScrollClass() = default;
	MessageScrollClass(int id, int x, int y, int width, int height);

	~MessageScrollClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Clicked(WWKey* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier) override;

	void DrawShape() const;

public:

	bool Hovering { false };
	int ID { 0 };
	int LastY { 0 };
	int LastScroll { 0 };
};