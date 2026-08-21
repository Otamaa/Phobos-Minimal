#pragma once
#include <GadgetClass.h>

class SelectedButtonClass : public GadgetClass
{
public:
	SelectedButtonClass() = default;
	SelectedButtonClass(int id, int x, int y);

	~SelectedButtonClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier) override;

	void DrawInfo() const;

	int ID { 0 };
	bool Hovering { false };
};

class SelectedNotButtonClass : public GadgetClass
{
public:
	SelectedNotButtonClass() = default;
	SelectedNotButtonClass(int id, int x, int y);

	~SelectedNotButtonClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;

	void DrawInfo() const;

	int ID { 0 };
	bool Hovering { false };
};

class SelectedToggleClass : public GadgetClass
{
public:
	SelectedToggleClass() = default;
	SelectedToggleClass(int id, int x, int y);

	~SelectedToggleClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier) override;

	void DrawInfo() const;

	int ID { 0 };
	bool Hovering { false };
};

class SelectedScrollClass : public GadgetClass
{
public:
	SelectedScrollClass() = default;
	SelectedScrollClass(int id, int x, int y);

	~SelectedScrollClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier) override;

	void DrawInfo() const;

	int ID { 0 };
	bool Hovering { false };
};
