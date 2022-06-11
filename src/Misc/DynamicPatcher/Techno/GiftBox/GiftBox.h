#pragma once
#ifdef COMPILE_PORTED_DP_FEATURES
#include <GeneralStructures.h>
#include <Utilities/SavegameDef.h>

class GiftBoxData;
class GiftBox
{
public:

	bool IsOpen;
	int Delay;
	TimerStruct DelayTimer;

	GiftBox(int delay) :
		IsOpen { false }
		, Delay { delay }
		, DelayTimer { }
	{
		if (delay > 0)
			DelayTimer.Start(delay);
	}

	GiftBox() :
		  IsOpen { false }
		, Delay { 0 }
		, DelayTimer { }
	{}

	bool CanOpen()
	{
		return !IsOpen && Timeup();
	}

	bool Timeup()
	{
		if (Delay <= 0 || DelayTimer.Expired())
		{
			IsOpen = true;
			return true;
		}
		return false;
	}

	void Reset(int nDelay)
	{
		IsOpen = false;
		Delay = nDelay;

		if (Delay > 0)
			DelayTimer.Start(nDelay);

	}

	void Release(TechnoClass* pOwner , GiftBoxData& nData);
	CellClass* GetCell(CellClass* pIn , CoordStruct& InOut, size_t nSpread, bool EmptyCell);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<GiftBox*>(this)->Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::Log("Processing Element From GiftBox ! \n");

		return Stm
			.Process(this->IsOpen)
			.Process(this->Delay)
			.Process(this->DelayTimer)
			.Success()
			;
	}

};

template <>
struct Savegame::ObjectFactory<GiftBox> {
	std::unique_ptr<GiftBox> operator() (PhobosStreamReader& Stm) const {
		return std::make_unique<GiftBox>();
	}
};

#endif