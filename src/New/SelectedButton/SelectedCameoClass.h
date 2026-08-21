#pragma once
#include <GadgetClass.h>

class SelectedCameoClass : public GadgetClass
{
public:
	SelectedCameoClass() = default;
	SelectedCameoClass(int id, int x, int y);

	~SelectedCameoClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier) override;

	void DrawInfo() const;

	int ID { 0 };
	bool Hovering { false };
};

class SelectedMainCameoClass : public GadgetClass
{
public:
	SelectedMainCameoClass() = default;
	SelectedMainCameoClass(int x, int y);

	~SelectedMainCameoClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;

	bool Hovering { false };
};
