#pragma once

#include "MessageToggleClass.h"

class MessageButtonClass : public MessageToggleClass
{
public:
	static constexpr int HoldInitialDelay = 30;
	static constexpr int HoldTriggerDelay = 5;

	MessageButtonClass() = default;
	MessageButtonClass(int id, int x, int y, int width, int height);

	~MessageButtonClass() = default;

	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override;

	void DrawShape() const;
public:

	int ID { 0 };
	int CheckTime { 0 };
};