#pragma once

#include <string>
#include <map>
#include <vector>
#include <any>
#include <typeinfo>
#include <type_traits>

#include <GeneralDefinitions.h>
#include <CellClass.h>
#include <MapClass.h>

#include <Phobos.CRT.h>
#include <Utilities/Parser.h>

static std::map<std::string, Mission> MissionTypeStrings
{
	{ "none", Mission::None }, // -1
	{ "sleep", Mission::Sleep }, // 0
	{ "attack", Mission::Attack }, // 1
	{ "move", Mission::Move }, // 2
	{ "qmove", Mission::QMove }, // 3
	{ "retreat", Mission::Retreat }, // 4
	{ "guard", Mission::Guard }, // 5
	{ "sticky", Mission::Sticky }, // 6
	{ "enter", Mission::Enter }, // 7
	{ "capture", Mission::Capture }, // 8
	{ "eaten", Mission::Eaten }, // 9
	{ "harvest", Mission::Harvest }, // 10
	{ "areaguard", Mission::Area_Guard }, // 11 AreaGuard
	{ "area_guard", Mission::Area_Guard }, // 11
	{ "return", Mission::Return }, // 12
	{ "stop", Mission::Stop }, // 13
	{ "ambush", Mission::Ambush }, // 14
	{ "hunt", Mission::Hunt }, // 15
	{ "deploy", Mission::Unload }, // 16 Deploy
	{ "unload", Mission::Unload }, // 16
	{ "sabotage", Mission::Sabotage }, //17
	{ "construction", Mission::Construction }, // 18
	{ "selling", Mission::Selling }, // 19
	{ "repair", Mission::Repair }, // 20
	{ "rescue", Mission::Rescue }, // 21
	{ "missile", Mission::Missile }, // 22
	{ "harmless", Mission::Harmless }, // 23
	{ "open", Mission::Open }, // 24
	{ "patrol", Mission::Patrol }, // 25
	{ "paradropApproach", Mission::ParadropApproach }, // 26
	{ "paradropOverfly", Mission::ParadropOverfly }, // 27
	{ "wait", Mission::Wait }, // 28
	{ "attackMove", Mission::AttackMove }, // 29
	{ "spyplaneApproach", Mission::SpyplaneApproach }, // 30
	{ "spyplaneOverfly", Mission::SpyplaneOverfly } // 31
};

template <>
inline bool Parser<Mission>::TryParse(const char* pValue, Mission* outValue)
{
	std::string key = PhobosCRT::lowercase(std::string(pValue));
	auto it = MissionTypeStrings.find(key);
	if (it != MissionTypeStrings.end())
	{
		*outValue = it->second;
		return true;
	}
	return false;
}

static std::map<std::string, LandType> LandTypeStrings
{
	{ "Clear", LandType::Clear },
	{ "Road", LandType::Road },
	{ "Water", LandType::Water },
	{ "Rock", LandType::Rock },
	{ "Wall", LandType::Wall },
	{ "Tiberium", LandType::Tiberium },
	{ "Beach", LandType::Beach },
	{ "Rough", LandType::Rough },
	{ "Ice", LandType::Ice },
	{ "Railroad", LandType::Railroad },
	{ "Tunnel", LandType::Tunnel },
	{ "Weeds", LandType::Weeds }
};

template <>
inline bool Parser<LandType>::TryParse(const char* pValue, LandType* outValue)
{
	std::string key = pValue;
	auto it = LandTypeStrings.find(key);
	if (it != LandTypeStrings.end())
	{
		*outValue = it->second;
		return true;
	}
	return false;
}

static std::map<std::string, TileType> TileTypeStrings
{
	{ "Tunnel", TileType::Tunnel },
	{ "Water", TileType::Water },
	{ "Blank", TileType::Blank },
	{ "Ramp", TileType::Ramp },
	{ "Cliff", TileType::Cliff },
	{ "Shore", TileType::Shore },
	{ "Wet", TileType::Wet },
	{ "MiscPave", TileType::MiscPave },
	{ "Pave", TileType::Pave },
	{ "DirtRoad", TileType::DirtRoad },
	{ "PavedRoad", TileType::PavedRoad },
	{ "PavedRoadEnd", TileType::PavedRoadEnd },
	{ "PavedRoadSlope", TileType::PavedRoadSlope },
	{ "Median", TileType::Median },
	{ "Bridge", TileType::Bridge },
	{ "WoodBridge", TileType::WoodBridge },
	{ "ClearToSandLAT", TileType::ClearToSandLAT },
	{ "Green", TileType::Green },
	{ "NotWater", TileType::NotWater },
	{ "DestroyableCliff", TileType::DestroyableCliff }
};

template <>
inline bool Parser<TileType>::TryParse(const char* pValue, TileType* outValue)
{
	std::string key = pValue;
	auto it = TileTypeStrings.find(key);
	if (it != TileTypeStrings.end())
	{
		*outValue = it->second;
		return true;
	}
	return false;
}

static std::map<std::string, DoType> SequenceStrings
{
	{ "Ready", DoType::Ready },
	{ "Guard", DoType::Guard },
	{ "Prone", DoType::Prone },
	{ "Walk", DoType::Walk },
	{ "FireUp", DoType::FireUp },
	{ "Down", DoType::Down },
	{ "Crawl", DoType::Crawl },
	{ "Up", DoType::Up },
	{ "FireProne", DoType::FireProne },
	{ "Idle1", DoType::Idle1 },
	{ "Idle2", DoType::Idle2 },
	{ "Die1", DoType::Die1 },
	{ "Die2", DoType::Die2 },
	{ "Die3", DoType::Die3 },
	{ "Die4", DoType::Die4 },
	{ "Die5", DoType::Die5 },
	{ "Tread", DoType::Tread },
	{ "Swim", DoType::Swim },
	{ "WetIdle1", DoType::WetIdle1 },
	{ "WetIdle2", DoType::WetIdle2 },
	{ "WetDie1", DoType::WetDie1 },
	{ "WetDie2", DoType::WetDie2 },
	{ "WetAttack", DoType::WetAttack },
	{ "Hover", DoType::Hover },
	{ "Fly", DoType::Fly },
	{ "Tumble", DoType::Tumble },
	{ "FireFly", DoType::FireFly },
	{ "Deploy", DoType::Deploy },
	{ "Deployed", DoType::Deployed },
	{ "DeployedFire", DoType::DeployedFire },
	{ "DeployedIdle", DoType::DeployedIdle },
	{ "Undeploy", DoType::Undeploy },
	{ "Cheer", DoType::Cheer },
	{ "Paradrop", DoType::Paradrop },
	{ "AirDeathStart", DoType::AirDeathStart },
	{ "AirDeathFalling", DoType::AirDeathFalling },
	{ "AirDeathFinish", DoType::AirDeathFinish },
	{ "Panic", DoType::Panic },
	{ "Shovel", DoType::Shovel },
	{ "Carry", DoType::Carry },
	{ "SecondaryFire", DoType::SecondaryFire },
	{ "SecondaryProne", DoType::SecondaryProne }
};

template <>
inline bool Parser<DoType>::TryParse(const char* pValue, DoType* outValue)
{
	std::string key = pValue;
	auto it = SequenceStrings.find(key);
	if (it != SequenceStrings.end())
	{
		*outValue = it->second;
		return true;
	}
	return false;
}

/// <summary>
/// 储存一个Section在一个ini文件中的全部KV对
/// </summary>
class INIBuffer
{
public:
	INIBuffer() {}
	INIBuffer(std::string fileName, std::string section)
	{
		this->FileName = fileName;
		this->Section = section;
	}

	void ClearBuffer()
	{
		Unparsed.clear();
		Parsed.clear();
	}

	bool GetUnparsed(std::string key, std::string& outValue)
	{
		auto it = Unparsed.find(key);
		if (it != Unparsed.end())
		{
			outValue = it->second;
			return true;
		}
		return false;
	}

	template <typename OutType>
	bool GetParsed(std::string key, OutType& outValue)
	{
		auto it = Parsed.find(key);
		if (it != Parsed.end())
		{
			outValue = std::any_cast<OutType>(it->second);
			return true;
		}
		auto ite = Unparsed.find(key);
		if (ite != Unparsed.end())
		{
			std::string value = ite->second;
			OutType buffer = {};
			if (Parse<OutType>(value.c_str(), &buffer))
			{
				std::any v = buffer;
				Parsed[key] = v;
				outValue = buffer;
				return true;
			}
		}
		return false;
	}

	// ----------------
	// 类型转换模板
	// ----------------

	template <typename OutType>
	inline size_t Parse(const char* value, OutType* outValue)
	{
		return Parser<OutType>::Parse(value, outValue);
	}

	template <>
	inline size_t Parse<CoordStruct>(const char* value, CoordStruct* outValue)
	{
		return Parser<int, 3>::Parse(value, (int*)outValue);
	}

	template <>
	inline size_t Parse<ColorStruct>(const char* value, ColorStruct* outValue)
	{
		return Parser<BYTE, 3>::Parse(value, (BYTE*)outValue);
	}

	// BulletVelocity
	template <>
	inline size_t Parse<Vector3D<double>>(const char* value, Vector3D<double>* outValue)
	{
		return Parser<double, 3>::Parse(value, (double*)outValue);
	}

	template <>
	inline size_t Parse<Point2D>(const char* value, Point2D* outValue)
	{
		return Parser<int, 2>::Parse(value, (int*)outValue);
	}

	template <>
	inline size_t Parse<Vector2D<double>>(const char* value, Vector2D<double>* outValue)
	{
		return Parser<double, 2>::Parse(value, (double*)outValue);
	}

	// CellStruct
	template<>
	inline size_t Parse<Vector2D<short>>(const char* value, Vector2D<short>* outValue)
	{
		return Parser<short, 2>::Parse(value, (short*)outValue);
	}

	template <typename OutType>
	bool GetParsedList(std::string key, std::vector<OutType>& outValues)
	{
		auto it = Parsed.find(key);
		if (it != Parsed.end())
		{
			outValues = std::any_cast<std::vector<OutType>>(it->second);
			return true;
		}
		auto ite = Unparsed.find(key);
		if (ite != Unparsed.end())
		{
			std::string v = ite->second;
			char str[_readLength];
			size_t length = v.copy(str, std::string::npos);
			str[length] = '\0';
			char* context = nullptr;
			std::vector<OutType> values = {};
			for (auto pCur = strtok_s(str, _readDelims, &context); pCur; pCur = strtok_s(nullptr, _readDelims, &context))
			{
				OutType buffer = {};
				if (Parser<OutType>::Parse(pCur, &buffer))
				{
					values.push_back(buffer);
				}
			}
			if (!values.empty())
			{
				std::any any = values;
				Parsed[key] = any;
				outValues = values;
				return true;
			}
		}
		return false;
	}

	std::string FileName{};
	std::string Section{};
	// 没有转换类型的kv对
	std::map<std::string, std::string> Unparsed{};
	// 已经装换了类型的kv对
	std::map<std::string, std::any> Parsed{};

private:
	static const size_t _readLength = 2048;
	const char _readDelims[4] = ",";
};

/// <summary>
/// 一个Section在多个ini文件中的KV对，按文件读取顺序连接。
/// 按顺序检索key，并返回首个符合的目标。
/// </summary>
class INILinkedBuffer
{
public:
	INILinkedBuffer() {}
	INILinkedBuffer(INIBuffer* buffer, INILinkedBuffer* nextBuffer)
	{
		m_Buffer = buffer;
		m_LinkedBuffer = nextBuffer;
	}

	auto operator<=>(const INILinkedBuffer&) const = default;

	std::vector<std::string> GetDependency()
	{
		std::vector<std::string> dependency;
		GetDependency(dependency);
		return dependency;
	}

	std::string GetSection()
	{
		return m_Buffer->Section;
	}

	void ClearBuffer() {}
	void Expired()
	{
		this->_expired = true;
	}
	bool IsExpired()
	{
		return this->_expired;
	}

	/// <summary>
	/// 返回首个持有key的buffer
	/// </summary>
	/// <param name="key"></param>
	/// <returns></returns>
	INIBuffer* GetFirstOccurrence(const char* key)
	{
		auto it = m_Buffer->Unparsed.find(key);
		if (it != m_Buffer->Unparsed.end())
		{
			return m_Buffer;
		}
		if (m_LinkedBuffer)
		{
			return m_LinkedBuffer->GetFirstOccurrence(key);
		}
		return nullptr;
	}

	// 读取未经转换的value
	bool GetUnparsed(const char* key, std::string& val)
	{
		if (m_Buffer->GetUnparsed(key, val))
		{
			return true;
		}
		if (m_LinkedBuffer)
		{
			return m_LinkedBuffer->GetUnparsed(key, val);
		}
		return false;
	}

	// 将Value转换成指定的type
	template <typename T>
	bool GetParsed(const char* key, T& outValue)
	{
		if (m_Buffer->GetParsed<T>(key, outValue))
		{
			return true;
		}
		if (m_LinkedBuffer)
		{
			return m_LinkedBuffer->GetParsed<T>(key, outValue);
		}
		return false;
	}

	// 读取多次value
	template <typename T>
	bool GetParsedList(const char* key, std::vector<T>& outValue)
	{
		if (m_Buffer->GetParsedList<T>(key, outValue))
		{
			return true;
		}
		if (m_LinkedBuffer)
		{
			return m_LinkedBuffer->GetParsedList<T>(key, outValue);
		}
		return false;
	}

private:
	bool _expired = false;

	void GetDependency(std::vector<std::string>& dependency)
	{
		dependency.push_back(m_Buffer->FileName);
		if (m_LinkedBuffer)
		{
			m_LinkedBuffer->GetDependency(dependency);
		}
	}

	INIBuffer* m_Buffer{};
	INILinkedBuffer* m_LinkedBuffer{};
};
