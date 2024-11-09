#pragma once

#include <Helpers\EnumFlags.h>
#include <TranslateFixedPoints.h>
#include <bit>
#include <Fixed.h>
#include <YRMath.h>
#include <CoordStruct.h>

enum class FacingType : char
{
	North = 0,
	NorthEast = 1,
	East = 2,
	SouthEast = 3,
	South = 4,
	SouthWest = 5,
	West = 6,
	NorthWest = 7,

	limit = 7,
	Count = 8,
	Min = 0,
	Max = 8,
	None = -1,
};


enum class DirType32 : unsigned char
{
	Min = 0,
	Max = 32,
};

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

	constexpr DirStruct(const DirStruct& nDir) noexcept :
		Raw { nDir.Raw }
	{  }

	constexpr explicit DirStruct() noexcept :
		Raw { 0 }
	{ }

	constexpr explicit DirStruct(int raw) noexcept :
		Raw { (unsigned short)raw }
	{ }

	explicit DirStruct(double rad) noexcept : Raw { 0 }
	{ SetRadian<65536>(rad); }

	constexpr explicit DirStruct(const DirType dir) noexcept : Raw { 0 }
	{ SetDir(dir); }

	constexpr explicit DirStruct(const FacingType face) noexcept :
		Raw { ((unsigned short)((unsigned char)face << 13 )) }
	{ }

	constexpr explicit DirStruct(const DirType32 face) noexcept :
		Raw { ((unsigned short)((unsigned char)face << 11)) }
	{}

	constexpr explicit DirStruct(size_t bits, const DirType value) noexcept : Raw { 0 }
	{ SetDir(bits, (unsigned short)(value)); }

	constexpr explicit DirStruct(const noinit_t&) noexcept
	{ }

	explicit DirStruct(double Y, double X) noexcept : Raw { 0 }
	{ SetRadian<65536>(Math::atan2(Y, X)); }

	explicit DirStruct(int x1, int y1, int x2, int y2) noexcept : Raw { 0 }
	{ SetRadian<65536>(Math::atan2((double)y2 - y1, (double)x2 - x1)); }


	constexpr FORCEINLINE bool operator==(const DirStruct& another) const
	{
		return this->Raw == another.Raw;
	}

	constexpr FORCEINLINE bool operator!=(const DirStruct& another) const
	{
		return this->Raw != another.Raw;
	}

	constexpr FORCEINLINE DirStruct& operator /= (const short nFace) {
		 this->Raw /= nFace;
		return *this;
	}

	constexpr FORCEINLINE DirStruct& operator += (const DirStruct& rhs) {
		this->Raw += rhs.Raw;
		return *this;
	}

	constexpr FORCEINLINE DirStruct& operator -= (const DirStruct& rhs) {
		this->Raw -= rhs.Raw;
		return *this;
	}

	constexpr FORCEINLINE DirStruct operator - (const DirStruct& rhs) const {
		return DirStruct(*this) -= rhs;
	}

	//constexpr FORCEINLINE DirStruct operator - () const {
	//	return DirStruct(-this->Raw);
	//}

	//DirStruct operator + () const {
	//	return *this;
	//}

	constexpr bool CompareToTwoDir(DirStruct& pBaseDir, DirStruct& pDirFrom) { return Math::abs(pDirFrom.Raw) >= Math::abs(this->Raw - pBaseDir.Raw); }

	DirStruct* GetDirOver(CoordStruct* coord1, CoordStruct* coord2)
		;// 	{ JMP_THIS(0x4265B0); }

	constexpr void Func_5B29C0(DirStruct& pDir2, DirStruct& pDir3) {

		if (Math::abs(pDir3.Raw) < Math::abs(this->Raw - pDir2.Raw))
		{
			if ((pDir2.Raw - this->Raw) >= 0)
			{
				this->Raw += this->Raw + pDir3.Raw;
			}
			else
			{
				this->Raw -= pDir3.Raw;
			}
		}
		else
		{
			this->Raw = pDir2.Raw;
		}
	}

	constexpr FORCEINLINE void SetDir(DirType dir) { Raw = ((unsigned short)((unsigned char)dir * 256)); }
	constexpr FORCEINLINE DirType GetDirFixed() const { return (DirType)((((this->Raw / (32768 / 256)) + 1) / 2) & (int)DirType::Max); }
	constexpr FORCEINLINE DirType GetDir() const { return (DirType)(this->Raw / 256); }
	constexpr FORCEINLINE void SetDir(size_t bit, size_t val) {
		if (bit <= 16u)
			Raw = ((unsigned short)TranslateFixedPoint::Normal(bit, 16u, val));
	}

	// If you want to divide it into 32 facings, as 32 has 5 bits
	// then you should type <5> here.
	// So does the others.
	template<size_t Bits>
	constexpr FORCEINLINE  size_t GetValue(size_t offset = 0) const
	{
		return TranslateFixedPoint::TemplatedCompileTime<16, Bits>(this->Raw, offset);
	}

	constexpr FORCEINLINE  size_t Getvalue8() const {
		return this->GetValue<3>();
	}

	constexpr FORCEINLINE  size_t Getvalue32() const {
		return this->GetValue<5>();
	}

	constexpr FORCEINLINE  size_t Getvalue256() const {
		return this->GetValue<8>();
	}

	size_t GetValue(size_t Bits = 16, size_t offset = 0) const {
		if (Bits <= 16u)
			return TranslateFixedPoint::Normal(16, Bits, this->Raw, offset);

		return 0;
	}

	void SetValue(short value, size_t Bits = 16, size_t offset = 0) {
		if (Bits > 0 && Bits <= 16) {
			Raw = static_cast<unsigned short>(TranslateFixedPoint::Normal(16, Bits, (size_t)value, offset));
		}
	}

	template<size_t Bits>
	constexpr FORCEINLINE  void SetValue(size_t value, size_t offset = 0)
	{
		Raw = ((unsigned short)(TranslateFixedPoint::TemplatedCompileTime<Bits, 16>(value, offset)));
	}

	template<size_t Count>
	constexpr FORCEINLINE  size_t GetFacing(size_t offset = 0) const
	{
		static_assert(std::has_single_bit(Count));

		constexpr size_t Bits = std::bit_width(Count - 1);
		return this->GetValue<Bits>(offset);
	}

	template<size_t Count>
	constexpr FORCEINLINE  void SetFacing(size_t value, size_t offset = 0)
	{
		static_assert(std::has_single_bit(Count));

		constexpr size_t Bits = std::bit_width(Count - 1);
		SetValue<Bits>(value, offset);
	}

	template <size_t FacingCount = 16>
	constexpr double GetRadian() const
	{
		static_assert(std::has_single_bit(FacingCount));

		constexpr size_t Bits = std::bit_width(FacingCount - 1);
		//constexpr size_t Max = (1 << Bits) - 1;

		size_t value = GetValue<Bits>();
		int dir = static_cast<int>(value) - FacingCount / 4; // LRotate 90 degrees
		return dir * (-Math::TwoPi / FacingCount);
	}

	template <size_t FacingCount = 16>
	constexpr void SetRadian(double rad)
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
	unsigned short Pad {};
};

static_assert(sizeof(DirStruct) == 4, "Invalid Size !");