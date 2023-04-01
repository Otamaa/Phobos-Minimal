#pragma once

#include <Helpers\EnumFlags.h>
#include <TranslateFixedPoints.h>
#include <bit>
#include <Fixed.h>
#include <YRMath.h>

enum class DirType : unsigned char
{
	North = 0,
	NorthEast = 32,
	East = 64,
	SouthEast = 96,
	South = 128,
	SouthWest = 160,
	West = 192,
	NorthWest = 224,

	Min = 0,
	Max = 255,

	HarvesterUnloadingA = 152,      // Direction of harvester while unloading.
	HarvesterUnloadingB = 144,      // Direction of harvester while unloading.
};
MAKE_ENUM_FLAGS(DirType);

// North -> 0x0000
// South -> 0x8000
// ...
// Just a very simple BAM
struct DirStruct
{
public:

	DirStruct(const DirStruct& nDir) noexcept :
		Raw { nDir.Raw }
	{  }

	explicit DirStruct() noexcept :
		Raw { 0 }
	{ }

	explicit DirStruct(int raw) noexcept :
		Raw { (unsigned short)raw }
	{ }

	explicit DirStruct(double rad) noexcept : Raw { 0 }
	{ SetRadian<65536>(rad); }

	explicit DirStruct(const DirType dir) noexcept : Raw { 0 }
	{ SetDir(dir); }
	
	explicit DirStruct(size_t bits, const DirType value) noexcept : Raw { 0 }
	{ SetDir(bits, (unsigned short)(value)); }

	explicit DirStruct(const noinit_t&) noexcept
	{ }

	explicit DirStruct(double Y, double X) noexcept : Raw { 0 }
	{ SetRadian<65536>(std::atan2(Y, X)); }

	bool operator==(const DirStruct& another) const
	{
		return this->Raw == another.Raw;
	}

	bool operator!=(const DirStruct& another) const
	{
		return this->Raw != another.Raw;
	}

	DirStruct& operator /= (const short nFace) {
		 this->Raw /= nFace;
		return *this;
	}

	DirStruct& operator += (const DirStruct& rhs) {
		this->Raw += rhs.Raw;
		return *this;
	}

	DirStruct& operator -= (const DirStruct& rhs) {
		this->Raw -= rhs.Raw;
		return *this;
	}

	DirStruct operator - (const DirStruct& rhs) const {
		return DirStruct(*this) -= rhs;
	}

	DirStruct operator - () const {
		return DirStruct(-this->Raw);
	}

	DirStruct operator + () const {
		return *this;
	}

	//TODO : Check Casting
	bool CompareToTwoDir(DirStruct& pBaseDir, DirStruct& pDirFrom);

	//TODO : Check Casting
	void Func_5B29C0(DirStruct& pDir2, DirStruct& pDir3);

	void SetDir(DirType dir);
	DirType GetDirFixed() const;
	DirType GetDir() const;
	void SetDir(size_t bit, size_t val);
	
	// If you want to divide it into 32 facings, as 32 has 5 bits
	// then you should type <5> here.
	// So does the others.
	template<size_t Bits>
	constexpr size_t GetValue(size_t offset = 0) const
	{
		return TranslateFixedPoint::TemplatedCompileTime<16, Bits>(this->Raw, offset);
	}

	size_t GetValue(size_t Bits = 16, size_t offset = 0);

	template<size_t Bits>
	constexpr void SetValue(size_t value, size_t offset = 0)
	{
		Raw = ((unsigned short)(TranslateFixedPoint::TemplatedCompileTime<Bits, 16>(value, offset)));
	}

	template<size_t Count>
	constexpr size_t GetFacing(size_t offset = 0) const
	{
		static_assert(std::has_single_bit(Count));

		constexpr size_t Bits = std::bit_width(Count - 1);
		return this->GetValue<Bits>(offset);
	}

	template<size_t Count>
	constexpr void SetFacing(size_t value, size_t offset = 0)
	{
		static_assert(std::has_single_bit(Count));

		constexpr size_t Bits = std::bit_width(Count - 1);
		SetValue<Bits>(value, offset);
	}

	template <size_t FacingCount = 16>
	double GetRadian() const
	{
		static_assert(std::has_single_bit(FacingCount));

		constexpr size_t Bits = std::bit_width(FacingCount - 1);
		//constexpr size_t Max = (1 << Bits) - 1;

		size_t value = GetValue<Bits>();
		int dir = static_cast<int>(value) - FacingCount / 4; // LRotate 90 degrees
		return dir * (-Math::TwoPi / FacingCount);
	}

	template <size_t FacingCount = 16>
	void SetRadian(double rad)
	{
		static_assert(std::has_single_bit(FacingCount));

		constexpr size_t Bits = std::bit_width(FacingCount - 1);
		constexpr size_t Max = (1 << Bits) - 1;

		int dir = static_cast<int>(rad / (-Math::TwoPi / FacingCount));
		size_t value = dir + FacingCount / 4; // RRotate 90 degrees
		SetValue<Bits>(value & Max);
	}

	unsigned short Raw;

private:
	unsigned short Pad;
};

static_assert(sizeof(DirStruct) == 4, "Invalid Size !");