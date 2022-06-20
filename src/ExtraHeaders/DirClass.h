#pragma once
#include <Fixed.h>

#pragma warning(push)
#pragma warning(disable : 4244)
typedef enum DirType
{
	DIR_N = 0,          // 0
	DIR_NE = 1 << 5,    // 32
	DIR_E = 2 << 5,     // 64
	DIR_SE = 3 << 5,    // 96
	DIR_S = 4 << 5,     // 128
	DIR_SW = 5 << 5,    // 160
	DIR_W = 6 << 5,     // 192
	DIR_NW = 7 << 5,    // 224

	DIR_MIN = 0,
	DIR_MAX = 255,

	DIR_SW_X1 = DirType((5 << 5) - 8),  // 152      // Direction of harvester while unloading.
	DIR_SW_X2 = DirType((5 << 5) - 16), // 144      // Direction of harvester while unloading.
} DirType;
DEFINE_ENUMERATION_OPERATORS(DirType);

/**
 *  DirClass is in stages of 128 (0-32768) (128*256).
 */
#pragma pack(4)
typedef class DirClass : public Fixed
{
public:
	explicit DirClass() : Fixed(0) { }
	explicit DirClass(int raw) { Set_Raw(raw); }
	explicit DirClass(const DirType dir) { Set_Dir(dir); }
	explicit DirClass(const noinit_t& noinit) { }

	void Set_Dir(DirType dir) { Fixed::Set_Raw(unsigned(dir % DIR_MAX) * 256); }
	DirType Get_Dir() const { return (DirType)((((Fixed::Get_Raw() / (32768 / 256)) + 1) / 2) & 255); }

	bool CompareToTwoDir(const DirClass& a, const DirClass& b) { JMP_THIS(0x5B2990); }
	bool Func_5B29C0(const DirClass& a, const DirClass& b) { JMP_THIS(0x5B29C0); }

} DirClass;
#pragma pack()
#pragma warning(pop)