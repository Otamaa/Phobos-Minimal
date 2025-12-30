#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/VectorHelper.h>
#include <Utilities/Interfaces.h>


class SuperClass;
class SWFirerClass
{
public:
	SuperClass* SW;
	CDTimerClass deferment;
	CellStruct cell;
	bool playerControl;
	int oldstart;
	int oldleft;

	bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		return this->Serialize(stm);
	}

	bool Save(PhobosStreamWriter& stm) const
	{
		return const_cast<SWFirerClass*>(this)->Serialize(stm);
	}

	SWFirerClass() = default;

	SWFirerClass(SuperClass* SW, int deferment, CellStruct cell, bool playerControl, int oldstart, int oldleft);
	~SWFirerClass() = default;

private:
	template <typename T>
	bool Serialize(T& stm)
	{
		return stm
			.Process(this->SW)
			.Process(this->deferment)
			.Process(this->cell)
			.Process(this->playerControl)
			.Process(this->oldstart)
			.Process(this->oldleft)
			.Success();
	}
};

struct SWFirerManagerClass : public GlobalSaveable
{
	HelperedVector<SWFirerClass> Array;

public:
	SWFirerManagerClass() = default;
	virtual ~SWFirerManagerClass() = default;

	virtual bool SaveGlobal(json& root);
	virtual bool LoadGlobal(const json& root);
	virtual void Clear();

	void Update();
public:
	static SWFirerManagerClass Instance;
};