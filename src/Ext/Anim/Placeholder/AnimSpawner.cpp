#include "AnimSpawner.h"

#include <Ext/AnimType/Body.h>

//
//void AnimSpawner::PlayAnims(const AnimSpawnerDatas* pData, const CoordStruct& Coord, HouseClass* pOwner)
//{
//	//TODO
//	Debug::Log("AnimSpawner , Play Anims ! \n");
//}
//
//void AnimSpawner::InitSpawnDatas(AnimTypeClass* pFrom) {
//	if (!pFrom)
//		return;
//
//	if (!m_SpawnDatas)
//	{
//		auto const pExt = AnimTypeExt::ExtMap.Find(pFrom);
//		m_SpawnDatas = std::make_unique<AnimSpawnerDatas>(pExt->SpawnerDatas);
//	}
//}
//
//void AnimSpawner::OnUpdate()
//{
//	if (!m_SpawnDatas)
//		return;
//
//	if (m_SpawnDatas->m_TriggerOnStart && m_SpawnDatas->m_Count != 0)
//	{
//		if (!m_alreadyInitEd)
//		{
//			m_alreadyInitEd = true;
//			const int init_delay = m_SpawnDatas->GetInitDelay();
//
//			if (init_delay > 0) {
//				m_initialDelay.Start(init_delay);
//			}
//			else
//			{
//				m_initialDelay.Stop();
//			}
//		}
//
//		if ((m_SpawnDatas->m_Count < 0 || m_spawnCount < m_SpawnDatas->m_Count) 
//			&& m_initialDelay.Expired() && m_delay.Expired())
//		{
//			AnimSpawner::PlayAnims(m_SpawnDatas.get(), AttachedTo->GetCoords(), AttachedTo->Owner);
//			m_spawnCount++;
//			const int delay = m_SpawnDatas->GetDelay();
//
//			if (delay > 0) {
//				m_delay.Start(delay);
//			}
//		}
//	}
//	else
//	{
//		ResetLoopSpawns();
//	}
//}
//
//void AnimSpawner::OnNext(AnimTypeClass* pNext)
//{
//	if (!m_SpawnDatas)
//		return;
//
//	ResetLoopSpawns();
//	if (m_SpawnDatas->m_TriggerOnNext)
//	{
//		AnimSpawner::PlayAnims(m_SpawnDatas.get(), AttachedTo->GetCoords(), AttachedTo->Owner);
//	}
//}
//
//void AnimSpawner::OnInit(CoordStruct* pCoord)
//{
//	if (!m_SpawnDatas)
//		return;
//
//	if (m_SpawnDatas->m_TriggerOnInit)
//	{
//		AnimSpawner::PlayAnims(m_SpawnDatas.get(), *pCoord, AttachedTo->Owner);
//	}
//}
//
//void AnimSpawner::OnDone()
//{
//	if (!m_SpawnDatas)
//		return;
//
//	if (m_SpawnDatas->m_TriggerOnDone)
//	{
//		AnimSpawner::PlayAnims(m_SpawnDatas.get(), AttachedTo->GetCoords(), AttachedTo->Owner);
//	}
//}
//
//void AnimSpawner::OnLoop()
//{
//	if (!m_SpawnDatas)
//		return;
//
//	if (m_SpawnDatas->m_TriggerOnDone)
//	{
//		AnimSpawner::PlayAnims(m_SpawnDatas.get(), AttachedTo->GetCoords(), AttachedTo->Owner);
//	}
//}
//
//void AnimSpawner::ResetLoopSpawns()
//{
//	m_alreadyInitEd = false;
//	m_initialDelay.Stop();
//	m_delay.Stop();
//	m_spawnCount = 0;
//}
//
//bool AnimSpawner::Load(PhobosStreamReader& Stm, bool RegisterForChange)
//{
//	return Serialize(Stm);
//}
//
//bool AnimSpawner::Save(PhobosStreamWriter& Stm) const
//{
//	return const_cast<AnimSpawner*>(this)->Serialize(Stm);
//}
//
//template <typename T>
//bool AnimSpawner::Serialize(T& Stm)
//{
//	return Stm
//		.Process(AttachedTo)
//		.Process(m_SpawnDatas)
//		.Process(m_alreadyInitEd)
//		.Process(m_initialDelay)
//		.Process(m_delay)
//		.Process(m_spawnCount)
//		.Success()
//		;
//}