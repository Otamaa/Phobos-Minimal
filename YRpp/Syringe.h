/*
	SYRINGE.H
	---------
	Holds macros, structures, classes that are necessary to interact with Syringe correctly.
																						-pd
*/

#pragma once

#include <Base/Always.h>
#include <DebugLog.h>

class LimitedRegister
{
protected:
	DWORD data;

	WORD* wordData()
	{
		return reinterpret_cast<WORD*>(&this->data);
	}

	BYTE* byteData()
	{
		return reinterpret_cast<BYTE*>(&this->data);
	}

public:
	WORD Get16()
	{
		return *this->wordData();
	}

	template<typename T>
	inline T Get()
	{
		return *reinterpret_cast<T*>(&this->data);
	}

	template<typename T>
	inline void Set(T value)
	{
		this->data = DWORD(value);
	}

	void Set16(WORD value)
	{
		*this->wordData() = value;
	}
};

class ExtendedRegister : public LimitedRegister
{
public:
	BYTE Get8Hi()
	{
		return this->byteData()[1];
	}

	BYTE Get8Lo()
	{
		return this->byteData()[0];
	}

	void Set8Hi(BYTE value)
	{
		this->byteData()[1] = value;
	}

	void Set8Lo(BYTE value)
	{
		this->byteData()[0] = value;
	}
};

class StackRegister : public ExtendedRegister
{
public:
	template<typename T>
	inline T* lea(int byteOffset)
	{
		return reinterpret_cast<T*>(static_cast<DWORD>(this->data + static_cast<DWORD>(byteOffset)));
	}

	inline DWORD lea(int byteOffset)
	{
		return static_cast<DWORD>(this->data + static_cast<DWORD>(byteOffset));
	}

	template<typename T>
	inline T At(int byteOffset)
	{
		return *reinterpret_cast<T*>(this->data + static_cast<DWORD>(byteOffset));
	}

	template<typename T>
	inline void At(int byteOffset, T value)
	{
		*reinterpret_cast<T*>(this->data + static_cast<DWORD>(byteOffset)) = value;
	}
};

//Macros to make the following a lot easier
#define REG_SHORTCUTS(reg) \
	inline DWORD reg() \
		{ return this->_ ## reg.Get<DWORD>(); } \
	template<typename T> inline T reg() \
		{ return this->_ ## reg.Get<T>(); } \
	template<typename T> inline void reg(T value) \
		{ this->_ ## reg.Set(value); } \

#define REG_SHORTCUTS_X(r) \
	DWORD r ## X() \
		{ return this->_E ## r ## X.Get16(); } \
	void r ## X(WORD value) \
		{ this->_E ## r ## X.Set16(value); } \

#define REG_SHORTCUTS_HL(r) \
	DWORD r ## H() \
		{ return this->_E ## r ## X.Get8Hi(); } \
	void r ## H(BYTE value) \
		{ this->_E ## r ## X.Set8Hi(value); } \
	DWORD r ## L() \
		{ return this->_E ## r ## X.Get8Lo(); } \
	void r ## L(BYTE value) \
		{ this->_E ## r ## X.Set8Lo(value); } \

#define REG_SHORTCUTS_XHL(r) \
	REG_SHORTCUTS_X(r); \
	REG_SHORTCUTS_HL(r); \

static constexpr const char* Register_names[] = {
  "EDI", "ESI", "EBP", "ESP", "EBX", "EDX", "ECX", "EAX"
};

enum class RegistersType : int {
	EDI , ESI , EBP , ESP , EBX , EDX , ECX , EAX
};

#define MAKEREG(RegName , inherit , type)\
struct RegName : public inherit {\
	constexpr inline const char* name() const {\
		return Register_names[(int)RegistersType::type];\
	}\
	constexpr inline RegistersType type() const {\
		return RegistersType::type;\
	}\
};\
static_assert(sizeof(RegName) == sizeof(inherit));

MAKEREG(LI_EDI, LimitedRegister, EDI)
MAKEREG(LI_ESI, LimitedRegister, ESI)
MAKEREG(ST_EBP, StackRegister, EBP)
MAKEREG(ST_ESP, StackRegister, ESP)
MAKEREG(EX_EBX, ExtendedRegister, EBX)
MAKEREG(EX_EDX , ExtendedRegister, EDX)
MAKEREG(EX_ECX, ExtendedRegister, ECX)
MAKEREG(EX_EAX, ExtendedRegister, EAX)

//A pointer to this class is passed as an argument to EXPORT functions
class REGISTERS
{
private:
	DWORD	origin;
	DWORD	flags;

	LI_EDI _EDI;
	LI_ESI _ESI;
	ST_EBP _EBP;
	ST_ESP _ESP;
	EX_EBX _EBX;
	EX_EDX _EDX;
	EX_ECX _ECX;
	EX_EAX _EAX;

public:
	DWORD Origin() const
	{
		return this->origin;
	}

	DWORD Origin(DWORD value)
	{
		this->origin = value;
		return this->origin;
	}

	DWORD EFLAGS() const
	{
		return this->flags;
	}

	void EFLAGS(DWORD value)
	{
		this->flags = value;
	}

	REG_SHORTCUTS(EAX);
	REG_SHORTCUTS(EBX);
	REG_SHORTCUTS(ECX);
	REG_SHORTCUTS(EDX);
	REG_SHORTCUTS(ESI);
	REG_SHORTCUTS(EDI);
	REG_SHORTCUTS(ESP);
	REG_SHORTCUTS(EBP);

	REG_SHORTCUTS_XHL(A);
	REG_SHORTCUTS_XHL(B);
	REG_SHORTCUTS_XHL(C);
	REG_SHORTCUTS_XHL(D);

	template<typename T>
	inline T lea_Stack(int offset)
	{
		return reinterpret_cast<T>(this->_ESP.lea(offset));
	}

	template<>
	inline DWORD lea_Stack(int offset)
	{
		return this->_ESP.lea(offset);
	}

	template<>
	inline int lea_Stack(int offset)
	{
		return static_cast<int>(this->_ESP.lea(offset));
	}

	template<typename T>
	inline T& ref_Stack(int offset)
	{
		return *this->lea_Stack<T*>(offset);
	}

	template<typename T>
	inline T Stack(int offset)
	{
		return this->_ESP.At<T>(offset);
	}

	DWORD Stack32(int offset)
	{
		return this->_ESP.At<DWORD>(offset);
	}

	WORD Stack16(int offset)
	{
		return this->_ESP.At<WORD>(offset);
	}

	BYTE Stack8(int offset)
	{
		return this->_ESP.At<BYTE>(offset);
	}

	template<typename T>
	inline T Base(int offset)
	{
		return this->_EBP.At<T>(offset);
	}

	template<typename T>
	inline void Stack(int offset, T value)
	{
		this->_ESP.At(offset, value);
	}

	void Stack16(int offset, WORD value)
	{
		this->_ESP.At(offset, value);
	}

	void Stack8(int offset, BYTE value)
	{
		this->_ESP.At(offset, value);
	}

	template<typename T>
	inline void Base(int offset, T value)
	{
		this->_EBP.At(offset, value);
	}
};

//Use this for DLL export functions
//e.g. EXPORT FunctionName(REGISTERS* R)
#define EXPORT extern "C" __declspec(dllexport) DWORD __cdecl
#define EXPORT_DEBUG_DECLARE(name) DWORD __forceinline name(REGISTERS* R);
#define EXPORT_DEBUG(name) DWORD __forceinline name(REGISTERS* R)
#define EXPORT_FUNC(name) extern "C" __declspec(dllexport) DWORD __cdecl name (REGISTERS *R)
#define EXPORT_FUNC_NAKED(name) extern "C" __declspec(naked , dllexport) DWORD __cdecl name (REGISTERS *R)

//Handshake definitions
struct SyringeHandshakeInfo
{
	int cbSize;
	int num_hooks;
	unsigned int checksum;
	DWORD exeFilesize;
	DWORD exeTimestamp;
	unsigned int exeCRC;
	int cchMessage;
	char* Message;
};

#define SYRINGE_HANDSHAKE(pInfo) extern "C" __declspec(dllexport) HRESULT __cdecl SyringeHandshake(SyringeHandshakeInfo* pInfo)

#if SYR_VER == 2

#pragma pack(push, 16)
#pragma warning(push)
#pragma warning( disable : 4324)
__declspec(align(16)) struct hookdecl
{
	unsigned int hookAddr;
	unsigned int hookSize;
	const char* hookName;
};

struct alignas(16) overridehookdecl
{
	unsigned int hookAddr;
	unsigned int hookSize;
	const char* hookNamePtr;
	const char* overrideModuleName;
};

//struct alignas(16) patchdecl
//{
//	unsigned int patchAddr;
//	BYTE* patchData;
//	unsigned int patchDataSize;
//	const char* hookNamePtr;
//};

__declspec(align(16)) struct hostdecl
{
	unsigned int hostChecksum;
	const char* hostName;
};
#pragma warning(pop)
#pragma pack(pop)

#pragma section(".syhks00", read, write)
//#pragma section(".syhks01", read, write)
//#pragma section(".syhks02", read, write)
//#pragma section(".syexe00", read, write)
namespace SyringeData
{
	namespace Hooks
	{

	};
	namespace Hosts
	{

	};
};

//#define declhost(exename, checksum) \
//namespace SyringeData { \
//namespace Hosts { \
//__declspec(allocate(".syexe00")) \
//hostdecl _hst__ ## exename = { checksum, #exename }; \
//}; };

#define declhook(hook, funcname, size) \
namespace SyringeData { \
namespace Hooks { \
__declspec(allocate(".syhks00"))\
hookdecl _hk__ ## hook ## funcname = { ## hook, ## size, #funcname }; \
}; };

//#define decl_override_hook(hook, funcname, size) \
//namespace SyringeData { \
//namespace Hooks { \
//__declspec(allocate(".syhks01"))\
//overridehookdecl _hk__ ## hook ## funcname = { ## hook, ## size, #funcname , "Ares.dll" }; \
//}; };

#endif // SYR_VER == 2


// create empty macros
#ifndef declhost
#define declhost(exename, checksum)
#endif // declhost

#ifndef declhook
#define declhook(hook, funcname, size)
#endif // declhook

//#ifndef decl_override_hook
//#define decl_override_hook(hook, funcname, size)
//#endif // declhook

//#define DEFINE_RAW_PATCH(hook,funcname ,...)								 \
//	namespace patch##funcname													 \
//	{																		 \
//		const BYTE data[] = {__VA_ARGS__};									 \
//	   __declspec(allocate(".syhks02"))										 \
//		patchdecl patch = { ##hook, (BYTE*)##data , sizeof(##data), #funcname};	 \
//	}
//
//#define DEFINE_RAW_PATCH_MANUAL(hook,funcname ,size , data)			\
//	namespace patch##funcname											\
//	{																\
//	   __declspec(allocate(".syhks02"))								\
//		patchdecl patch = { ##hook, (BYTE*)##data , ##size, #funcname};	\
//	}

//#define DEBUG_HOOK

#ifndef DEBUG_HOOK
#define DEFINE_HOOK(hook,funcname,size) \
declhook(hook, funcname, size) \
EXPORT_FUNC(funcname)

// Does the same as DEFINE_HOOK but no function opening, use for injgen-declaration when repeating the same hook at multiple addresses.
// CAUTION: funcname must be the same as in DEFINE_HOOK.
#define DEFINE_HOOK_AGAIN(hook, funcname, size) \
declhook(hook, funcname, size)

#else
#include <chrono>
struct DebugData
{
	static std::chrono::steady_clock::time_point StartTime;
	static void Start(DWORD origin, const char* funcName, int size);
	static void End(DWORD origin, const char* funcName, int size);
	static void StartO(DWORD origin, const char* funcName, int size);
	static void EndO(DWORD origin, const char* funcName, int size);
};

#define DEFINE_HOOK(hook, funcname, size) \
declhook(hook, funcname##_DEBUG_HOOK__LOG_, size) \
EXPORT_DEBUG_DECLARE(funcname##_DEBUG_) \
EXPORT_FUNC(funcname##_DEBUG_HOOK__LOG_) \
{\
DebugData::Start(R->Origin(), #funcname , size);\
DWORD ret=funcname##_DEBUG_(R);\
DebugData::End(R->Origin(), #funcname, size);\
return ret;\
}\
EXPORT_DEBUG(funcname##_DEBUG_)

#define DEFINE_HOOK_AGAIN(hook, funcname, size) \
declhook(hook, funcname##_DEBUG_HOOK__LOG_, size)

#endif

#pragma region DONOTLOG
//#define DEFINE_STRONG_OVERRIDE_HOOK(hook,funcname,size) \
//declhook(hook, funcname, size) \
//EXPORT_FUNC(funcname)

//decl_override_hook(hook, funcname, size)
//#define DEFINE_STRONG_OVERRIDE_HOOK_AGAIN(hook, funcname, size) \
//declhook(hook, funcname, size)

#define DEFINE_STRONG_HOOK(hook,funcname,size) \
declhook(hook, funcname, size) \
EXPORT_FUNC(funcname)

// Does the same as DEFINE_HOOK but no function opening, use for injgen-declaration when repeating the same hook at multiple addresses.
// CAUTION: funcname must be the same as in DEFINE_HOOK.
#define DEFINE_STRONG_HOOK_AGAIN(hook, funcname, size) \
declhook(hook, funcname, size)

#pragma endregion

//#ifndef DEBUG_HOOK
//decl_override_hook(hook, funcname, size)
//EXPORT_FUNC(funcname)
//#define DEFINE_OVERRIDE_HOOK(hook,funcname,size) \
//declhook(hook, funcname, size) \
//EXPORT_FUNC(funcname)\
//
//decl_override_hook(hook, funcname, size)
//#define DEFINE_OVERRIDE_HOOK_AGAIN(hook, funcname, size) \
//declhook(hook, funcname, size)
//
//#else
//
//#define DEFINE_OVERRIDE_HOOK(hook, funcname, size) \
//decl_override_hook(hook, funcname##_DEBUG_HOOK__LOG_, size) \
//EXPORT_DEBUG_DECLARE(funcname##_DEBUG_) \
//EXPORT_FUNC(funcname##_DEBUG_HOOK__LOG_) \
//{\
//DebugData::StartO(R->Origin(), #funcname , size);\
//DWORD ret=funcname##_DEBUG_(R);\
//DebugData::EndO(R->Origin(), #funcname, size);\
//return ret;\
//}\
//EXPORT_DEBUG(funcname##_DEBUG_)
//
//#define DEFINE_OVERRIDE_HOOK_AGAIN(hook, funcname, size) \
//decl_override_hook(hook, funcname##_DEBUG_HOOK__LOG_, size)
//
//#endif

//#define DEFINE_DISABLE_HOOK(hook,funcname)
//PRAGMA_DISABLEWARNING()
//PRAGMA_DISABLEWARNING_S(4245)
//PRAGMA_DISABLEWARNING_S(4838)
//decl_override_hook(hook, funcname, -1)
//PRAGMA_DISABLEWARNING_POP()

//#define DEFINE_OVERRIDE_SKIP_HOOK(hook,funcname,size,ret)\
//DEFINE_HOOK(hook,funcname,size){ return 0x ##ret## ;}
//DEFINE_OVERRIDE_HOOK(hook,funcname,size){ return 0x ##ret## ;}

#define DEFINE_SKIP_HOOK(hook,funcname,size,ret)\
DEFINE_HOOK(hook,funcname,size){ return 0x ##ret## ;}