#pragma once

#include <CRT.h>
#include <sstream>
#include <WarheadFlags.h>
#include <Helpers/Macro.h>

// converters
struct Conversions
{
	static double Str2Armor(const char *buf, WarheadFlags *whFlags) {

		double val = 0.0;
		if(strchr(buf, '%')) { // convert to double
			val = atoi(buf) * 0.01;
		} else {
			val = atof(buf);
		}

		const double nValCopy = std::abs(val);

		if (!nValCopy) { //0.0
			whFlags->ForceFire = false;
			whFlags->PassiveAcquire = false;
			whFlags->Retaliate = false;
		}
		else {

			//re-evaluate after this
			whFlags->ForceFire = true;
			whFlags->PassiveAcquire = true;
			whFlags->Retaliate = true;

			if (LESS_EQUAL(nValCopy, 0.02)) {
				whFlags->PassiveAcquire = false;
			}

			if (LESS_EQUAL(nValCopy, 0.01)) {
				whFlags->Retaliate = false;
			}
		}

		return val;
	}

	static bool IsValidFloat(const std::string& myString) {
		std::istringstream iss(myString);
		float f;
		iss >> std::noskipws >> f; // noskipws considers leading whitespace invalid
		// Check the entire string was consumed and if either failbit or badbit is set

		return iss && iss.eof() && !iss.fail();
	}

	static std::pair<bool,double> GetAndCheckDouble(const std::string& myString)
	{
		std::istringstream iss(myString.data());
		double f;
		iss >> std::noskipws >> f; // noskipws considers leading whitespace invalid
		// Check the entire string was consumed and if either failbit or badbit is set

		return { iss && iss.eof() && !iss.fail() , f};
	}

	static inline bool IsValidArmorValue(const char* buf) {

		if (!strlen(buf))
			return false;

		if (CRT::strchr(buf, '%')) {
			return true ;
		}
		else if(IsValidFloat(buf)) {
			return true ;
		}

		return false;
	}

	static inline std::pair<bool, double> Str2ArmorCheck(const char* buf, WarheadFlags* whFlags)
	{
		if (!buf || !whFlags || !IsValidArmorValue(buf))  {
			return { false ,0.0 };
		}

		return {true , Str2Armor(buf, whFlags) };
	}

	// narrow_cast(): a searchable way to do narrowing casts of values
	template <class T, class U>
	static inline T narrow_cast(U&& u) noexcept
	{
		return static_cast<T>(std::forward<U>(u));
	}

	// OMG OPTIMIZED:
	// http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
	static constexpr inline unsigned int Int2Highest(DWORD v) {
		unsigned int r; // result of log2(v) will go here
		unsigned int shift;

		r = static_cast<DWORD>(v > 0xFFFF) << 4; v >>= r;
		shift = static_cast<DWORD>(v > 0xFF) << 3; v >>= shift; r |= shift;
		shift = static_cast<DWORD>(v > 0xF) << 2; v >>= shift; r |= shift;
		shift = static_cast<DWORD>(v > 0x3) << 1; v >>= shift; r |= shift;
															   r |= (v >> 1);
		return r;
	}

	static constexpr inline unsigned int Int2Highest(int v) {
		return Int2Highest(static_cast<DWORD>(v));
	}

	static inline int Distance(int x1, int y1, int x2, int y2)
	{
		int diff1 = y1 - y2;
		if (diff1 < 0) diff1 = -diff1;
		int diff2 = x1 - x2;
		if (diff2 < 0) diff2 = -diff2;
		if (diff1 > diff2)
		{
			return(diff1 + ((unsigned)diff2 / 2));
		}
		return(diff2 + ((unsigned)diff1 / 2));
	}

};
