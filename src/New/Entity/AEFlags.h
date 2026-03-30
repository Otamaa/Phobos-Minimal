#pragma once

#include <Base/Always.h>

class PhobosStreamReader;
class PhobosStreamWriter;
struct AEFlags
{
	union
	{
		struct
		{
			unsigned Cloakable : 1;
			unsigned ForceDecloak : 1;

			unsigned DisableWeapons : 1;
			unsigned DisableSelfHeal : 1;

			unsigned HasRangeModifier : 1;
			unsigned HasTint : 1;
			unsigned HasOnFireDiscardables : 1;
			unsigned HasExtraWarheads : 1;
			unsigned HasFeedbackWeapon : 1;

			unsigned ReflectDamage : 1;
			unsigned Untrackable : 1;

			unsigned DisableRadar : 1;
			unsigned DisableSpySat : 1;
			unsigned Unkillable : 1;

			unsigned _reserved : 18;  // leave unused room for future flags
		};

		uint32_t bits; // raw access
	};

public:

	//load bit from int and flip the bits from it saved state
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	//write each bit as integer on .Process to ensure compatibility
	bool Save(PhobosStreamWriter& Stm) const;

private:
	static constexpr size_t BitCount = sizeof(bits) * CHAR_BIT;

};
