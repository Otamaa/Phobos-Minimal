#pragma once
#include <GadgetClass.h>

class MessageLabelClass : public GadgetClass
{
public:
	MessageLabelClass() = default;
	MessageLabelClass(int x, int y, size_t id, int deleteTime, bool animate, int drawDelay);

	~MessageLabelClass() = default;

	virtual bool Draw(bool bForced) override;

	const wchar_t* GetText() const;

public:

	size_t ID { 0 };
	int DeleteTime { 0 };
	bool Animate { false };
	size_t AnimPos { 0 };
	size_t AnimTiming { 0 };
	size_t DrawPos { 0 };
	int DrawDelay { 0 };
};