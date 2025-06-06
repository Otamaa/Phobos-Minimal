
#include <Ext/Tactical/Body.h>

#include <Utilities/Macro.h>

// Replace single call
DEFINE_FUNCTION_JUMP(CALL,0x4ABCEB, FakeTacticalClass::Tactical_MakeFilteredSelection);

// Replace vanilla function. For in case another module tries to call the vanilla function at offset
DEFINE_FUNCTION_JUMP(LJMP, 0x6D9FF0, FakeTacticalClass::Tactical_MakeFilteredSelection);
