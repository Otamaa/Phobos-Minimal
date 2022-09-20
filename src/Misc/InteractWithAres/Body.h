#pragma once
#include <Base/Always.h>

class TechnoClass;
class TechnoTypeClass;
class SuperWeaponTypeClass;

struct AresData
{
	static  HMODULE AresDllHmodule;
	struct HandleConvert
	{
		static DWORD Offset;
		static DWORD CallableAddress;

		static void __stdcall Exec(TechnoClass*, TechnoTypeClass*);
	};

	struct ContainerMapData
	{
		static DWORD SWTypeContainerOffSet;
		static DWORD SWTypeContainer;
	};

	struct ContainerMap_Find
	{
		static DWORD Offset;
		static DWORD CallableAddress;


		//template<typename T>
		//static void* __stdcall Exec(DWORD , T);

		//template<>
		static void* __fastcall Exec(DWORD* c, SuperWeaponTypeClass* p);
	};

	static void Init();
	static void UnInit()
	{
		FreeLibrary(AresDllHmodule);
	}
};

class Ares
{
private:
	static const uint32_t BaseAddress;

	template<uint32_t Address>
	static constexpr uint32_t get_addr()
	{ return Address >= 0x10000000 ? Address - 0x10000000 : Address; }

	template<uint32_t Address, typename Tret, typename... TArgs>
	struct AresStdcall
	{
		using fp_type = Tret(__stdcall*)(TArgs...);

		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(BaseAddress + get_addr<Address>())(args...);
		}
	};

	template<uint32_t Address, typename... TArgs>
	struct AresStdcall<Address, void, TArgs...>
	{
		using fp_type = void(__stdcall*)(TArgs...);

		void operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(BaseAddress + get_addr<Address>())(args...);
		}
	};

	template<uint32_t Address, typename Tret, typename... TArgs>
	struct AresCdecl
	{
		using fp_type = Tret(__cdecl*)(TArgs...);

		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(BaseAddress + get_addr<Address>())(args...);
		}
	};

	template<uint32_t Address, typename... TArgs>
	struct AresCdecl<Address, void, TArgs...>
	{
		using fp_type = void(__cdecl*)(TArgs...);

		void operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(BaseAddress + get_addr<Address>())(args...);
		}
	};

	template<uint32_t Address, typename Tret, typename... TArgs>
	struct AresFastcall
	{
		using fp_type = Tret(__fastcall*)(TArgs...);

		decltype(auto) operator()(TArgs... args) const
		{
			return reinterpret_cast<fp_type>(BaseAddress + get_addr<Address>())(args...);
		}
	};

	template<uint32_t Address, typename... TArgs>
	struct AresFastcall<Address, void, TArgs...>
	{
		using fp_type = void(__fastcall*)(TArgs...);

		void operator()(TArgs... args) const
		{
			reinterpret_cast<fp_type>(BaseAddress + get_addr<Address>())(args...);
		}
	};

	//template<uint32_t Address, typename Tret, typename TThis, typename... TArgs>
	//struct AresThiscall
	//{
	//	using fp_type = Tret(__fastcall*)(TThis, void*, TArgs...);

	//	decltype(auto) operator()(TThis pThis, TArgs... args) const
	//	{
	//		return reinterpret_cast<fp_type>(BaseAddress + get_addr<Address>())(pThis, nullptr, args...);
	//	}
	//};

	//template<uint32_t Address, typename TThis, typename... TArgs>
	//struct AresThiscall<Address, void, TThis, TArgs...>
	//{
	//	using fp_type = void(__fastcall*)(TThis, void*, TArgs...);

	//	void operator()(TArgs... args) const
	//	{
	//		reinterpret_cast<fp_type>(BaseAddress + get_addr<Address>())(pThis, nullptr, args...);
	//	}
	//};

public:
	static constexpr AresStdcall<0x10043CF0, bool, TechnoClass*, TechnoTypeClass*> TypeConversion {};
};