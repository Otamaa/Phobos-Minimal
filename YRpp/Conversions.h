#pragma once

#include <Helpers/Macro.h>
#include <YRPP.h>
#include <WarheadTypeClass.h>

#include <sstream>

// converters
class Conversions
{
public:
	static double Str2Armor(const char *buf, WarheadFlags *whFlags) {
		if(!buf) { return 0.0; }

		bool ForceFire = true;
		bool Retaliate = true;
		bool PassiveAcquire = true;

		if(CRT::strchr(buf, '%')) {

			if(CRT::strlen(buf) == 2) {
				switch(*buf) {
					case '0':
						ForceFire = false;
					case '1':
						Retaliate = false;
					case '2':
						PassiveAcquire = false;
						break;
				}
			}
			whFlags->ForceFire = ForceFire;
			whFlags->Retaliate = Retaliate;
			whFlags->PassiveAcquire = PassiveAcquire;
			return CRT::atoi(buf) * 0.01;
		} else {
			double vs = CRT::atof(buf);
			if(LESS_EQUAL(vs, 0.02)) {
				whFlags->PassiveAcquire = false;
			}
			if(LESS_EQUAL(vs, 0.01)) {
				whFlags->Retaliate = false;
			}
			if(LESS_EQUAL(vs, 0.00)) {
				whFlags->ForceFire = false;
			}
			return vs;
		}
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
		if (!buf || !whFlags) { return { false ,0.0 }; }

		if (CRT::strchr(buf, '%'))
		{
			if (CRT::strlen(buf) == 2)
			{
				switch (*buf)
				{
				case '0':
				{
					whFlags->ForceFire = false;
					whFlags->Retaliate = false;
					whFlags->PassiveAcquire = false;
				}
				break;
				case '1':
				{
					whFlags->ForceFire = true;
					whFlags->Retaliate = false;
					whFlags->PassiveAcquire = false;
				}
				break;
				case '2':
				{
					whFlags->ForceFire = true;
					whFlags->Retaliate = true;
					whFlags->PassiveAcquire = false;
					break;
				}
				}
			}
			else
			{

				whFlags->ForceFire = true;
				whFlags->Retaliate = true;
				whFlags->PassiveAcquire = true;
			}

			return { true , CRT::atoi(buf) * 0.01 };
		}
		else
		{
			const auto& [IsValid, vs] = GetAndCheckDouble(buf);

			if(!IsValid)
				return { false ,  0.0 };

			if (LESS_EQUAL(vs, 0.02))
			{
				whFlags->PassiveAcquire = false;
			}
			if (LESS_EQUAL(vs, 0.01))
			{
				whFlags->Retaliate = false;
			}
			if (LESS_EQUAL(vs, 0.00))
			{
				whFlags->ForceFire = false;
			}

			return {true , vs};
		}

		return { false ,  0.0 };
	}

	// narrow_cast(): a searchable way to do narrowing casts of values
	template <class T, class U>
	static inline T narrow_cast(U&& u) noexcept
	{
		return static_cast<T>(std::forward<U>(u));
	}

	// OMG OPTIMIZED:
	// http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
	static inline unsigned int Int2Highest(DWORD v) {
		unsigned int r; // result of log2(v) will go here
		unsigned int shift;

		r =     static_cast<DWORD>(v > 0xFFFF) << 4; v >>= r;
		shift = static_cast<DWORD>(v > 0xFF  ) << 3; v >>= shift; r |= shift;
		shift = static_cast<DWORD>(v > 0xF   ) << 2; v >>= shift; r |= shift;
		shift = static_cast<DWORD>(v > 0x3   ) << 1; v >>= shift; r |= shift;
		                                                          r |= (v >> 1);
		return r;
	}

	static inline unsigned int Int2Highest(int v) {
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
