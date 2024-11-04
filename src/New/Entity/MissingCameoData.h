#pragma once

#include <Ext/Rules/Body.h>
#include <Utilities/PhobosPCXFile.h>

struct MissingCameoData
{
	SHPStruct* m_SHP_Fallback;

	SHPStruct* m_SHP;
	PhobosPCXFile m_PCX;
	UniqueGamePtrB<BSurface> m_SHPToPcxSurface;

	~MissingCameoData()
	{
		m_PCX.~PhobosPCXFile();
		GameDelete<true>(m_SHP);
		GameDelete<true>(m_SHP_Fallback);
	}

	MissingCameoData() : m_SHP { nullptr }, m_PCX {}, m_SHPToPcxSurface {} {
		m_SHP_Fallback = (SHPStruct*)FileSystem::LoadFile(GameStrings::XXICON_SHP(), true);
	}

	void Initialize() {
		m_SHP = (SHPStruct*)FileSystem::LoadFile(RulesExtData::Instance()->MissingCameo.data(), true);
		m_PCX = RulesExtData::Instance()->MissingCameo;

		// TODO :
		// if pcx doesnt exist , make copy of it from shp
		// if shp exist but pcx dont , make copy of it from shp
	}

	BSurface* GetPCXSurface() {
		return this->m_PCX.GetSurface();
	}

	SHPStruct* GetSHPFile() {
		return !this->m_SHP ? this->m_SHP_Fallback : this->m_SHP;
	}
};