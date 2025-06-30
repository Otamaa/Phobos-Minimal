#pragma once

#include <Ext/Rules/Body.h>
#include <Utilities/PhobosPCXFile.h>
#include <Utilities/SHPWrapper.h>

struct MissingCameoData
{
	SHPWrapper m_SHP_Fallback;
	SHPWrapper m_SHP;
	PhobosPCXFile m_PCX;
	UniqueGamePtrC<BSurface> m_SHPToPcxSurface;

	MissingCameoData() : m_PCX {}, m_SHPToPcxSurface {} {
		// Load fallback SHP with automatic memory management
		m_SHP_Fallback = SHPWrapper(GameStrings::XXICON_SHP());
	}

	void Initialize() {
		// Load custom SHP with fallback to the default one
		m_SHP = SHPWrapper(RulesExtData::Instance()->MissingCameo.data(), m_SHP_Fallback.get());
		m_PCX = RulesExtData::Instance()->MissingCameo;

		// TODO :
		// if pcx doesnt exist , make copy of it from shp
		// if shp exist but pcx dont , make copy of it from shp
	}

	BSurface* GetPCXSurface() {
		return this->m_PCX.GetSurface();
	}

	SHPStruct* GetSHPFile() {
		// Return the custom SHP if valid, otherwise fallback
		return m_SHP ? m_SHP.get() : m_SHP_Fallback.get();
	}
};