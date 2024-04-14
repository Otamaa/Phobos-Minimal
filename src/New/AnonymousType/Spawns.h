#pragma once

#include <Utilities/TemplateDefB.h>
#include <Utilities/SavegameDef.h>

class Spawns
{
	static constexpr std::string title = "Spawns.";
public:
	bool Enable { false };

	ValueableVector<AnimTypeClass*> Anims {};
	ValueableVector<int> Nums {};
	ValueableVector<double> Chances {};
	Valueable<bool> RandomType { false };
	ValueableVector<int> RandomWeights {};

	Valueable<CoordStruct> Offset {};
	bool UseRandomOffset { false };
	Valueable<Point2D> RandomOffset {};
	bool UseRandomOffsetFLH { false };
	Valueable<Point2D> RandomOffsetF {};
	Valueable<Point2D> RandomOffsetL {};
	Valueable<Point2D> RandomOffsetH {};

	Valueable<bool> TriggerOnDone { true };
	Valueable<bool> TriggerOnNext { false };
	Valueable<bool> TriggerOnLoop { false };
	Valueable<bool> TriggerOnStart { false };
	Valueable<int> Count { 1 };

	Valueable<int> InitDelay { 0 };
	bool UseRandomInitDelay { false };
	Valueable<Point2D> RandomInitDelay {};

	Valueable<int> Delay { 0 };
	bool UseRandomDelay { false };
	Valueable<Point2D> RandomDelay {};

	void Read(INI_EX& reader, const char* pSection)
	{
		Anims.Read(reader, pSection, (title + "Anims").c_str(), true);
		Nums.Read(reader, pSection, (title + "Nums").c_str());
		Chances.Read(reader, pSection, (title + "Chances").c_str());

		RandomType.Read(reader, pSection, (title + "RandomType").c_str());
		RandomWeights.Read(reader, pSection, (title + "RandomWeights").c_str());

		Offset.Read(reader, pSection, (title + "Offset").c_str());
		RandomOffset.Read(reader, pSection, (title + "RandomOffset").c_str());
		UseRandomOffset = RandomOffset->IsValid();
		RandomOffsetF.Read(reader, pSection, (title + "RandomOffsetF").c_str());
		RandomOffsetL.Read(reader, pSection, (title + "RandomOffsetL").c_str());
		RandomOffsetH.Read(reader, pSection, (title + "RandomOffsetH").c_str());
		UseRandomOffsetFLH = RandomOffsetF->IsValid() || RandomOffsetL->IsValid() || !RandomOffsetH->IsValid();

		TriggerOnDone.Read(reader, pSection, (title + "TriggerOnDone").c_str());
		TriggerOnNext.Read(reader, pSection, (title + "TriggerOnNext").c_str());
		TriggerOnLoop.Read(reader, pSection, (title + "TriggerOnLoop").c_str());

		TriggerOnStart.Read(reader, pSection, (title + "TriggerOnStart").c_str());
		Count.Read(reader, pSection, (title + "Count").c_str());

		InitDelay.Read(reader, pSection, (title + "InitDelay").c_str());
		RandomInitDelay.Read(reader, pSection, (title + "RandomInitDelay").c_str());
		UseRandomInitDelay = RandomInitDelay->IsValid();

		Delay.Read(reader, pSection, (title + "Delay").c_str());
		RandomDelay.Read(reader, pSection, (title + "RandomDelay").c_str());
		UseRandomDelay = RandomDelay->IsValid();

		Enable = !Anims.empty() && (TriggerOnDone || TriggerOnNext || TriggerOnLoop || TriggerOnStart);

	}

	CoordStruct GetOffset()
	{
		if (UseRandomOffsetFLH)
		{
			int f = GeneralUtils::GetRandomValue(RandomOffsetF, 0);
			int l = GeneralUtils::GetRandomValue(RandomOffsetL, 0);
			int h = GeneralUtils::GetRandomValue(RandomOffsetH, 0);
			return { f, l, h };
		}
		else if (UseRandomOffset)
		{
			int min = RandomOffset->X;
			int max = RandomOffset->Y;

			if (max > 0)
			{
				return GeneralUtils::GetRandomOffset(min, max);
			}
		}

		return Offset;
	}

	int GetInitDelay()
	{
		if (UseRandomInitDelay)
		{
			return GeneralUtils::GetRandomValue(RandomInitDelay, 0);
		}
		return InitDelay;
	}

	int GetDelay()
	{
		if (UseRandomDelay)
		{
			return GeneralUtils::GetRandomValue(RandomDelay, 0);
		}
		return Delay;
	}

	void SpawnAnims(
		TechnoClass* pInvoker,
		HouseClass* pOwner,
		CoordStruct& where
	);

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->Enable)
			.Process(this->Anims)
			.Process(this->Nums)
			.Process(this->Chances)
			.Process(this->RandomType)
			.Process(this->RandomWeights)

			.Process(this->Offset)

			.Process(this->UseRandomOffset)
			.Process(this->RandomOffset)

			.Process(this->UseRandomOffsetFLH)
			.Process(this->RandomOffsetF)
			.Process(this->RandomOffsetL)
			.Process(this->RandomOffsetH)

			.Process(this->TriggerOnDone)
			.Process(this->TriggerOnNext)
			.Process(this->TriggerOnLoop)
			.Process(this->TriggerOnStart)
			.Process(this->Count)

			.Process(this->InitDelay)
			.Process(this->UseRandomInitDelay)
			.Process(this->RandomInitDelay)

			.Process(this->Delay)
			.Process(this->UseRandomDelay)
			.Process(this->RandomDelay)

			.Success();
	};

	bool Load(PhobosStreamReader& stream, bool registerForChange)
	{
		return this->Serialize(stream);
	}

	bool Save(PhobosStreamWriter& stream) const
	{
		return const_cast<Spawns*>(this)->Serialize(stream);
	}
#pragma endregion
};
