#pragma once

#include <map>
#include <CCINIClass.h>
#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDefB.h>
#include <Misc/KratosPP/Utils/Enums.h>

struct INIConfig
{
	bool Readed = false;
	bool Enable = false;

	virtual void Read(INI_EX& Dep , const char* pSection) = 0;

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->Enable)
			.Success();
	};

	virtual bool Load(PhobosStreamReader& stream, bool registerForChange)
	{
		return this->Serialize(stream);
	}
	virtual bool Save(PhobosStreamWriter& stream) const
	{
		return const_cast<INIConfig*>(this)->Serialize(stream);
	}
#pragma endregion
};

template <>
inline bool Parser<SubjectToGroundType>::TryParse(const char* pValue, SubjectToGroundType* outValue)
{
	switch (toupper(static_cast<unsigned char>(*pValue)))
	{
	case '1':
	case 'T':
	case 'Y':
		if (outValue)
		{
			*outValue = SubjectToGroundType::YES;
		}
		return true;
	case '0':
	case 'F':
	case 'N':
		if (outValue)
		{
			*outValue = SubjectToGroundType::NO;
		}
		return true;
	}
	return false;
}

struct TrajectoryData : INIConfig
{
	bool AdvancedBallistics { true };

	// Arcing
	int ArcingFixedSpeed { 0 }; // 恒定速度飞行，近距离高抛，远距离平抛
	bool Inaccurate { false }; // 不精确散布
	float BallisticScatterMin { 0 }; // 最小散布距离
	float BallisticScatterMax { 0 }; // 最大散布距离
	int Gravity { RulesClass::Instance->Gravity }; // 自定义重力

	// Straight
	bool Straight { false }; // 直线飞行
	bool AbsolutelyStraight { false }; // 朝向正面的直线飞行

	// Missile
	bool ReverseVelocity { false }; // 反转出膛飞行方向
	bool ReverseVelocityZ { false }; // 反转出膛飞行方向
	float ShakeVelocity { 0.0f }; // 出膛方向随机抖动

	// Status
	SubjectToGroundType SubjectToGround { SubjectToGroundType::AUTO };

	virtual void Read(INI_EX& Dep, const char* pSection) override
	{
		 detail::read<bool>(this->AdvancedBallistics ,Dep, pSection , "AdvancedBallistics");

		// Arcing
		 detail::read<int>(this->ArcingFixedSpeed, Dep, pSection, "Arcing.FixedSpeed");
		 detail::read<bool>(this->Inaccurate, Dep, pSection, "Inaccurate");
		 detail::read<float>(this->BallisticScatterMin, Dep, pSection, "BallisticScatter.Min");
		 detail::read<float>(this->BallisticScatterMax, Dep, pSection, "BallisticScatter.Max");

		 detail::read<int>(this->Gravity, Dep, pSection, "Gravity");

		// Straight
		 detail::read<bool>(this->Straight, Dep, pSection, "Straight");
		 detail::read<bool>(this->AbsolutelyStraight, Dep, pSection, "AbsolutelyStraight");

		// Missile
		 detail::read<bool>(this->ReverseVelocity, Dep, pSection, "ROT.Reverse");
		 detail::read<bool>(this->ReverseVelocityZ, Dep, pSection, "ROT.ReverseZ");
		 detail::read<float>(this->ShakeVelocity, Dep, pSection, "ROT.ShakeMultiplier");

		// Status
		 std::string _SubjectToGround;
		 detail::read<std::string>(_SubjectToGround, Dep, pSection, "SubjectToGround");
		 Parser<SubjectToGroundType>::TryParse(_SubjectToGround.c_str(), &this->SubjectToGround);
	}

	bool IsStraight()
	{
		return Straight || AbsolutelyStraight;
	}
};

struct ConfigsHolder
{

	template<typename T>
	T* GetData(CCINIClass* Dep, const char* pSection) {
		auto it = Data.find(Dep);
		 buffer_*;

		if (it == Data.end()) {
			it = Data.emplace(Dep, {}).first;
		}

		std::map<std::string, std::vector<std::unique_ptr<INIConfig>>>* buffer_ = &it->second;

		auto itb = buffer_->find(pSection);
		if (itb == buffer_->end()) {
			itb = buffer_->emplace(pSection, {}).first;
		}

		std::vector<std::unique_ptr<INIConfig> vecbuff_ = &itb->second;

		if (vecbuff_->empty()) {
			vecbuff_->push_back(std::move(std::make_unique<T>()));
			INI_EX dep_(Dep);
			vecbuff_->back()->Read(dep_, pSection);
		}

		T* ret = nullptr;
		for (auto& ptr : *vecbuff_) {
			if (typedef(ptr.get()) == typedef(&T)) {
				ret = ptr.get();
				break;
			}
		}

		return ret;
	}

	std::map<CCINIClass* , std::map<std::string , std::vector<std::unique_ptr<INIConfig>>>> Data {};
};