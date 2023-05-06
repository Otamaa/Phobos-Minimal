#include "AnimSpawnerDatas.h"

#include <Utilities/GeneralUtils.h>

void AnimSpawnerDatas::Read(INI_EX& exINI, const char* pID)
{
	//TODO : 
		// Controlled Parse (skip if it same name with the parent !)

	//Debug::Log("AnimSpawnerDatas , Read For [%s] ! \n" , pID);
}

int AnimSpawnerDatas::GetInitDelay()
{
	if (m_UseRandomInitDelay)
	{
		return GeneralUtils::GetRandomValue(m_RandomInitDelay, 0);
	}

	return m_InitDelay;
}

int AnimSpawnerDatas::GetDelay()
{
	if (m_UseRandomDelay)
	{
		return GeneralUtils::GetRandomValue(m_RandomDelay, 0);
	}

	return m_Delay;
}

bool AnimSpawnerDatas::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool AnimSpawnerDatas::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<AnimSpawnerDatas*>(this)->Serialize(Stm);
}

template <typename T>
bool AnimSpawnerDatas::Serialize(T& Stm)
{
	return Stm
		.Process(m_TriggerOnStart)
		.Process(m_TriggerOnNext)
		.Process(m_TriggerOnInit)
		.Process(m_TriggerOnDone)
		.Process(m_TriggerOnLoop)
		.Process(m_Count)
		.Process(m_InitDelay)
		.Process(m_UseRandomInitDelay)
		.Process(m_RandomInitDelay)
		.Process(m_Delay)
		.Process(m_UseRandomDelay)
		.Process(m_RandomDelay)
		.Success()
		;
}