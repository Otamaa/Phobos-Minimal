#pragma once

#include <SuperClass.h>
#include <Utilities/SavegameDef.h>

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

	SWFirerClass(SuperClass* SW, int deferment, CellStruct cell, bool playerControl, int oldstart, int oldleft) :
		SW { SW },
		deferment {},
		cell { cell },
		playerControl { playerControl },
		oldstart { oldstart },
		oldleft { std::max(oldleft - deferment, 0) }
	{
		this->SW->Reset();
		this->deferment.Start(deferment);
	}

	~SWFirerClass() = default;

public:
	inline static HelperedVector<SWFirerClass> Array;

	static void Clear() { Array.clear(); }
	static void Update();

	static bool SaveGlobals(PhobosStreamWriter& stm) {
		return stm
			.Process(Array)
			.Success();
	}

	static bool LoadGlobals(PhobosStreamReader& stm) {
		return stm
			.Process(Array)
			.Success();
	}

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