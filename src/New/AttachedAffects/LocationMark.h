#pragma once
#include <Utilities/SavegameDef.h>

struct LocationMark
{
	CoordStruct m_Location {};
	DirStruct m_Direction {};

	LocationMark() = default;
	LocationMark(const CoordStruct& location, const DirStruct& direction) noexcept :
		m_Location { location }
		, m_Direction { direction }
	{ }

	~LocationMark() = default;

	inline bool Load(PhobosStreamReader& stm, bool RegisterForChange)
	{
		return stm
			.Process(m_Location)
			.Process(m_Direction)
			.Success()
			&& stm.RegisterChange(this) // announce this type
			;
	}

	inline bool Save(PhobosStreamWriter& stm)
	{
		return stm
			.Process(m_Location)
			.Process(m_Direction)
			.Success()
			 && stm.RegisterChange(this) // announce this type
			;
	}
};