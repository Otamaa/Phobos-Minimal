#pragma once

#include <Utilities/SavegameDef.h>
#include "AttachEffect.h"

class AttachEffectManager {
	std::vector<std::unique_ptr<AttachEffect>> m_AEs {};
};