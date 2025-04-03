#pragma once

#include <cstring>

class PKey
{
public:
	PKey() : Modulus(), Exponent(), BitPrecision(0) { std::memset(Modulus, 0, 256); std::memset(Exponent, 0, 256); }

private:
	char Modulus[256];         // BigInt
	char Exponent[256];        // BigInt
	int BitPrecision;
};
