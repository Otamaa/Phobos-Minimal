#include <Phobos.h>
#include "AresChecksummer.h"

#include <Helpers/Macro.h>
#include <Utilities/Macro.h>


DEFINE_FUNCTION_JUMP(LJMP,
	0x4A1C10, CRCEngine::operator()<BYTE>);

DEFINE_FUNCTION_JUMP(LJMP,
	0x4A1CA0, CRCEngine::operator()<bool>);

DEFINE_FUNCTION_JUMP(LJMP,
	0x4A1D30, CRCEngine::operator()<WORD>);

DEFINE_FUNCTION_JUMP(LJMP,
	0x4A1D50, CRCEngine::operator()<DWORD>);

DEFINE_FUNCTION_JUMP(LJMP,
	0x4A1D70, CRCEngine::operator()< float >);

DEFINE_FUNCTION_JUMP(LJMP,
	0x4A1D90, CRCEngine::operator()<double>);

DEFINE_FUNCTION_JUMP(LJMP,
	0x4A1DE0, CRCEngine::Updata_external);