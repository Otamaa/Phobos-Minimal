#pragma once

#include <GeneralDefinitions.h>
#include <Fixed.h>

#pragma warning(push)
#pragma warning(disable : 4244)

/**
 *  DirClass is in stages of 128 (0-32768) (128*256).
 */
#pragma pack(4)
typedef class DirClass : public Fixed
{
public:
	explicit DirClass() : Fixed(0) { }
	explicit DirClass(int raw) { Set_Raw(raw); }
	explicit DirClass(const DirTypes dir) { Set_Dir(dir); }
	explicit DirClass(const noinit_t& noinit) { }

	void Set_Dir(DirTypes dir) { Fixed::Set_Raw(unsigned(dir % DIR_MAX) * 256); }
	DirTypes Get_Dir() const { return (DirTypes)((((Fixed::Get_Raw() / (32768 / 256)) + 1) / 2) & 255); }

	bool CompareToTwoDir(const DirClass& a, const DirClass& b) { JMP_THIS(0x5B2990); }
	bool Func_5B29C0(const DirClass& a, const DirClass& b) { JMP_THIS(0x5B29C0); }

} DirClass;
#pragma pack()
#pragma warning(pop)