#pragma once

#include <cstring>

class PKey
{
public:
	PKey() : Modulus(), Exponent(), BitPrecision(0) {
		__stosb(reinterpret_cast<unsigned char*>(Modulus), 0, 256);
		__stosb(reinterpret_cast<unsigned char*>(Exponent), 0, 256);

	}

private:
	char Modulus[256];         // BigInt
	char Exponent[256];        // BigInt
	int BitPrecision;
};