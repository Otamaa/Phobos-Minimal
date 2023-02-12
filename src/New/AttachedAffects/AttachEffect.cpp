#include "AttachEffect.h"

#include <Utilities/GeneralUtils.h>
#include "AttachEffectType.h"

#include "Affects/Stand/Stand.h"

AttachEffect::AttachEffect(AttachEffectType* pType) : m_Type { pType }
	, m_AttachedTo { nullptr }
	, m_Source { nullptr }
	, m_SourceHouse { nullptr }

	, m_DetonateLoc { }
	, m_FromWarhead { false }

	, m_AEMode { -1 }
	, m_FromPassenger { false }

	, m_NonInheritable { false }

	, m_active { false }
	, m_Duration { 0 }
	, m_Isimmortal { false }

	, m_LifeTimer { }
	, m_InitialDelayTimer { }
	, m_IsdelayToEnable { false }

	, m_Effects { }
{
	Init();
}

void AttachEffect::SetupLifeTimer()
{
	if (!m_Isimmortal) {
		m_LifeTimer.Start(m_Duration);
	}
}

void AttachEffect::Enable(TechnoClass* pSource, HouseClass* pSourceHouse, const CoordStruct& warheadLocation, int aeMode, bool fromPassenger)
{
	m_active = true;
	m_Source = pSource;
	m_SourceHouse = pSourceHouse;
	m_DetonateLoc = warheadLocation;
	m_FromWarhead = warheadLocation != CoordStruct::Empty;
	m_AEMode = aeMode;
	m_FromPassenger = fromPassenger;

	if (!m_IsdelayToEnable || m_InitialDelayTimer.Expired())
		EnableEffects();

}

void AttachEffect::EnableEffects()
{
	m_IsdelayToEnable = false;
	SetupLifeTimer();

	for(auto const& effect : m_Effects)
	{
		if (!effect)
			continue;

		effect->Enable(this);
	}
}

void AttachEffect::Init()
{

	const int initDelay = GeneralUtils::GetRandomValue(m_Type->InitialRandomDelay,m_Type->InitialDelay);

	m_IsdelayToEnable = initDelay > 0;

	if (m_IsdelayToEnable) {
		m_InitialDelayTimer.Start(initDelay);
	}

	m_Duration = m_Type->Duration;
	m_Isimmortal = m_Type->HoldDuration;

	if(auto const& pStandType = m_Type->Stand)
	m_Effects.push_back(std::move(std::make_unique<Stand>(pStandType.get())));



}