#pragma once

#include <CoordStruct.h>
#include <Utilities/TemplateDef.h>

//struct AnimSpawnerDatas;
//class HouseClass;
//class AnimTypeClass;
//class AnimClass;
//class AnimSpawner
//{
//	AnimClass* AttachedTo;
//	std::unique_ptr<AnimSpawnerDatas> m_SpawnDatas;
//	bool m_alreadyInitEd;
//	TimerStruct m_initialDelay;
//	TimerStruct m_delay;
//	int m_spawnCount;
//
//public:
//	static void PlayAnims(const AnimSpawnerDatas* pData, const CoordStruct& Coord, HouseClass* pOwner);
//
//	AnimSpawner() = default;
//	AnimSpawner(AnimClass* pOwner) noexcept :
//		  AttachedTo {  pOwner }
//		, m_SpawnDatas { }
//		, m_alreadyInitEd { false }
//		, m_initialDelay { }
//		, m_delay { }
//		, m_spawnCount { 0 }
//
//	{
//		InitSpawnDatas(pOwner->Type);
//	}
//
//	virtual ~AnimSpawner() = default;
//
//	void OnUpdate();
//	void OnNext(AnimTypeClass* pNext);
//	void OnInit(CoordStruct* pCoord);
//	void OnDone();
//	void OnLoop();
//
//	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
//	bool Save(PhobosStreamWriter& Stm) const;
//
//	template <typename T>
//	bool Serialize(T& Stm);
//
//private:
//
//	void InitSpawnDatas(AnimTypeClass* pFrom);
//	void ResetLoopSpawns();
//};