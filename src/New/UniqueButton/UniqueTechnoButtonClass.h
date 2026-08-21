#pragma once
#include <GadgetClass.h>

class UniqueTechnoButtonClass : public GadgetClass
{
public:
	UniqueTechnoButtonClass() = default;
	UniqueTechnoButtonClass(int id, int x, int y);

	~UniqueTechnoButtonClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier) override;

	int ID { 0 };
	bool Hovering { false };
};
