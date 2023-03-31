#pragma once

#include <Helpers\EnumFlags.h>
#include <TranslateFixedPoints.h>
#include <bit>
#include <Fixed.h>

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
struct DirStruct : public Fixed
{
public:

	DirStruct(const DirStruct& nDir) noexcept :
		Fixed { nDir.Get_Raw() }
	{  }

	DirStruct() noexcept :
		Fixed { 0 }
	{ }

	DirStruct(int raw) noexcept :	
		Fixed { (unsigned short)raw }
	{ }

	DirStruct(double rad) noexcept : Fixed { 0 }
	{ SetRadian<65536>(rad); }

	DirStruct(const DirType dir) noexcept : Fixed { 0 }
	{ SetDir(dir); }
	
	DirStruct(size_t bits, const DirType value) noexcept : Fixed { 0 }
	{
		SetDir(bits, (unsigned short)(value));
	}

	DirStruct(const noinit_t&) noexcept : Fixed { noinit_t() }
	{ }

	unsigned short Raw() const { return this->Data.Raw; }

	DirStruct(double Y, double X) noexcept : Fixed { 0 }
	{ SetRadian<65536>(std::atan2(Y, X)); }

	bool operator==(const DirStruct& another) const
	{
		return this->Data.Raw == another.Data.Raw;
	}

	bool operator!=(const DirStruct& another) const
	{
		return this->Data.Raw != another.Data.Raw;
	}

	DirStruct& operator /= (const short nFace) {
		 this->Data.Raw /= nFace;
		return *this;
	}

	DirStruct& operator += (const DirStruct& rhs) {
		this->Data.Raw += rhs.Data.Raw;
		return *this;
	}

	DirStruct& operator -= (const DirStruct& rhs) {
		this->Data.Raw -= rhs.Data.Raw;
		return *this;
	}

	DirStruct operator - (const DirStruct& rhs) const {
		return DirStruct(*this) -= rhs;
	}

	DirStruct operator - () const {
		return DirStruct(-this->Data.Raw);
	}

	DirStruct operator + () const {
		return *this;
	}

	//TODO : Check Casting
	bool CompareToTwoDir(DirStruct& pBaseDir, DirStruct& pDirFrom)
		{ return std::abs(pDirFrom.Data.Raw) >= std::abs(this->Data.Raw - pBaseDir.Data.Raw); }

	//TODO : Check Casting
	void Func_5B29C0(DirStruct& pDir2, DirStruct& pDir3) {

		if (std::abs(pDir3.Data.Raw) < std::abs(this->Data.Raw - pDir2.Data.Raw)) {
			if ((pDir2.Data.Raw - this->Data.Raw) >= 0) {
				this->Data.Raw += this->Data.Raw + pDir3.Data.Raw;
			} else {
				this->Data.Raw -= pDir3.Data.Raw;
			}
		}
		else {
			this->Data.Raw = pDir2.Data.Raw;
		}
	}

	void SetDir(DirType dir) {
		this->Set_Raw((unsigned short)((int)dir * 256));
	}

	DirType GetDirFixed() const {
		return (DirType)((((this->Data.Raw / (32768 / 256)) + 1) / 2) & (int)DirType::Max);
	}

	DirType GetDir() const {
		return (DirType)(this->Data.Raw / 256);
	}

	void SetDir(size_t bit, size_t val) {
		if (bit <= 16u)
			this->Set_Raw((unsigned short)TranslateFixedPoint::Normal(bit, 16u, val));
	}
	
	// If you want to divide it into 32 facings, as 32 has 5 bits
	// then you should type <5> here.
	// So does the others.
	template<size_t Bits>
	constexpr size_t GetValue(size_t offset = 0) const
	{
		return TranslateFixedPoint::TemplatedCompileTime<16, Bits>(this->Data.Raw, offset);
	}

	unsigned short GetValue(size_t Bits = 16 , size_t offset = 0)
	{
		if (Bits <= 16u)
			return (unsigned short)(TranslateFixedPoint::Normal(16, Bits, this->Data.Raw, offset));

		return 0;
	}

	template<size_t Bits>
	constexpr void SetValue(size_t value, size_t offset = 0)
	{
		this->Set_Raw((unsigned short)(TranslateFixedPoint::TemplatedCompileTime<Bits, 16>(value, offset)));
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
};

static_assert(sizeof(DirStruct) == 4, "Invalid Size !");