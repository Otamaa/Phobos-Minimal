#pragma once

#include <GeneralDefinitions.h>
#include <TranslateFixedPoints.h>
enum class DirType : unsigned char;

// North -> 0x0000
// South -> 0x8000
// ...
// Just a very simple BAM
struct DirStruct
{
public:
	explicit DirStruct() noexcept : Raw { 0 } { }
	explicit DirStruct(int raw) noexcept : Raw { static_cast<unsigned short>(raw) } { }
	explicit DirStruct(double rad) noexcept { SetRadian<65536>(rad); }
	explicit DirStruct(const DirType dir) noexcept { SetDir(dir); }
	explicit DirStruct(double Y, double X) : DirStruct() {
		SetRadian<65536>(Math::atan2(Y, X));
	}

	explicit DirStruct(const noinit_t& noinit) noexcept { }

	bool operator==(const DirStruct& another) const
	{
		return this->Raw == another.Raw;
	}

	bool operator!=(const DirStruct& another) const
	{
		return !(*this == another);
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
	bool CompareToTwoDir(DirStruct& pBaseDir, DirStruct& pDirFrom)
		{ return abs(pDirFrom.Raw) >= abs(Raw - pBaseDir.Raw); }

	//TODO : Check Casting
	bool Func_5B29C0(DirStruct& pDir2, DirStruct& pDir3)
	{
		if (abs(pDir3.Raw) < abs(Raw - pDir2.Raw))
		{
			if ((pDir2.Raw - Raw) >= 0)
			{
				Raw += Raw + pDir3.Raw;
			}
			else
			{
				Raw -= pDir3.Raw;
			}

			return false;
		}
		else
		{
			Raw = pDir2.Raw;
			return true;
		}
	}

	void SetDir(DirType dir)
	{
		Raw = static_cast<unsigned short>(static_cast<unsigned char>(dir) * 256);
	}

	DirType Get_Dir() const
	{
		return static_cast<DirType>(Raw / 256);
	}

	// If you want to divide it into 32 facings, as 32 has 5 bits
	// then you should type <5> here.
	// So does the others.
	template<size_t Bits>
	constexpr size_t GetValue(size_t offset = 0) const
	{
		return this->TranslateFixedPoint<16, Bits>(Raw, offset);
	}

	unsigned short GetValue(size_t Bits = 16)
	{
		if (Bits > 0 && Bits <= 16)
			return (unsigned short)(TranslateFixedPointNoconstexpr(16, Bits, Raw, 0));

		return 0;
	}

	template<size_t Bits>
	constexpr void SetValue(size_t value, size_t offset = 0)
	{
		Raw = static_cast<unsigned short>(this->TranslateFixedPoint<Bits, 16>(value, offset));
	}

	template<size_t Count>
	constexpr size_t GetFacing(size_t offset = 0) const
	{
		static_assert(HasSingleBit(Count));

		constexpr size_t Bits = this->BitWidth<Count - 1>();
		return this->GetValue<Bits>(offset);
	}

	template<size_t Count>
	constexpr void SetFacing(size_t value, size_t offset = 0)
	{
		static_assert(HasSingleBit(Count));

		constexpr size_t Bits = this->BitWidth<Count - 1>();
		SetValue<Bits>(value, offset);
	}

	template <size_t FacingCount = 16>
	double GetRadian() const
	{
		static_assert(HasSingleBit(FacingCount));

		constexpr size_t Bits = BitWidth<FacingCount - 1>();
		//constexpr size_t Max = (1 << Bits) - 1;

		size_t value = GetValue<Bits>();
		int dir = static_cast<int>(value) - FacingCount / 4; // LRotate 90 degrees
		return dir * (-Math::TwoPi / FacingCount);
	}

	template <size_t FacingCount = 16>
	void SetRadian(double rad)
	{
		static_assert(HasSingleBit(FacingCount));

		constexpr size_t Bits = BitWidth<FacingCount - 1>();
		constexpr size_t Max = (1 << Bits) - 1;

		int dir = static_cast<int>(rad / (-Math::TwoPi / FacingCount));
		size_t value = dir + FacingCount / 4; // RRotate 90 degrees
		SetValue<Bits>(value & Max);
	}

private:

	constexpr static bool HasSingleBit(size_t x) noexcept
	{
		return x != 0 && (x & (x - 1)) == 0;
	}

	template<size_t X>
	constexpr static size_t BitWidth() noexcept
	{
		if constexpr (X == 0)
			return 0;

		size_t T = X;
		size_t cnt = 0;
		while (T)
		{
			T >>= 1;
			++cnt;
		}

		return cnt;
	}

	template<size_t BitsFrom, size_t BitsTo>
	constexpr static size_t TranslateFixedPoint(size_t value, size_t offset = 0)
	{
		constexpr size_t MaskIn = ((1u << BitsFrom) - 1);
		constexpr size_t MaskOut = ((1u << BitsTo) - 1);

		if constexpr (BitsFrom > BitsTo)
			return (((((value & MaskIn) >> (BitsFrom - BitsTo - 1)) + 1) >> 1) + offset) & MaskOut;
		else if constexpr (BitsFrom < BitsTo)
			return (((value - offset) & MaskIn) << (BitsTo - BitsFrom)) & MaskOut;
		else
			return value & MaskOut;
	}

public:
	unsigned short Raw;
private:
	unsigned short Padding;
};

static_assert(sizeof(DirStruct) == 4);