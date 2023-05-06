#pragma once

#include <Utilities/TemplateDef.h>

struct AnimSpawnerDatas
{
	bool m_TriggerOnStart;
	bool m_TriggerOnNext;
	bool m_TriggerOnInit;
	bool m_TriggerOnDone;
	bool m_TriggerOnLoop;
	int m_Count;

	int m_InitDelay;
	bool m_UseRandomInitDelay;
	Point2D m_RandomInitDelay;

	int m_Delay;
	bool m_UseRandomDelay;
	Point2D m_RandomDelay;


	int GetInitDelay();

	int GetDelay();

	void Read(INI_EX& exINI, const char* pID);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	template <typename T>
	bool Serialize(T& Stm);
};