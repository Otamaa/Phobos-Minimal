
#include <string>
#include <Utilities/Patch.h>

enum class HookType : int
{
	None = -1 ,
	/// <summary>Specifies that the hook is a ares style hook.</summary>
	AresHook,
	/// <summary>Specifies that the hook is just a jump to destination.</summary>
	/// <remarks>This type hook can not hook a hooked address.</remarks>
	SimpleJumpToRet,
	/// <summary>Specifies that the hook is just a jump to hook.</summary>
	/// <remarks>This type hook can not hook a hooked address.</remarks>
	DirectJumpToHook,
	/// <summary>Specifies that the hook is to write bytes to address.</summary>
	/// <remarks>This type hook can not hook a hooked address.</remarks>
	WriteBytesHook,

	/// <summary>Specifies that the hook is to overwrite exported target reference address.</summary>
	ExportTableHook,
	/// <summary>Specifies that the hook is to overwrite imported target reference address.</summary>
	ImportTableHook
};

class HookAttribute
{
private:

	HookType Type { HookType::None };
	uintptr_t Address { 0 };
	uintptr_t RelativeAddress { 0 };
	int Size { -1 };
	std::string Module {};
	std::string TargetName {};

public:

	HookAttribute(HookType type, uintptr_t addr , int size,  std::string modulestr) :
		Type { type }, Module { modulestr }
	{
		if(type != HookType::ExportTableHook && type != HookType::ImportTableHook)
		{

			if (!modulestr.empty()) {
				_address = addr;
			} else {
				// set RelativeAddress
				_address = addr - Patch::ModuleDatas[Module.c_str()].BaseAddr;
			}

			_size = size;
		}
	}

	uintptr_t GetAddress() const
	{
		switch (Type)
		{
		case HookType::ExportTableHook:
			return Patch::GetEATAddress(Module.c_str(), TargetName.c_str());
		case HookType::ImportTableHook:
			return Patch::GetIATAddress(Module.c_str(), TargetName.c_str());
		}

		if (!Module.empty()) {
			return _address;
		}

		return (uintptr_t)Patch::ModuleDatas[Module.c_str()].BaseAddr + RelativeAddress;
	}

	uintptr_t GetRelativeAddress() const
	{
		switch (Type)
		{
		case HookType::ExportTableHook:
		case HookType::ImportTableHook:
			return Address - Patch::ModuleDatas[Module.c_str()].BaseAddr;
		}

		if (Module.empty())
		{
			return Address - Patch::ModuleDatas[Module.c_str()].BaseAddr;
		}

		return _address;
	}

	int GetSize() const
	{
		switch (Type)
		{
		case HookType::ExportTableHook:
		case HookType::ImportTableHook:
			return sizeof(uint);
		}

		return _size;
	}

private:
	int _address { -1 };
	int _size { -1 };
};