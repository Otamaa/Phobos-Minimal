#pragma once

#include <cstring>

class PKey
{
public:
	PKey() : Modulus(), Exponent(), BitPrecision(0) {
		__stosb(reinterpret_cast<unsigned char*>(Modulus), 0, 256);
		__stosb(reinterpret_cast<unsigned char*>(Exponent), 0, 256);

	}
	
	~PKey() = default;

	int GetBitPrecision() const { return this->BitPrecision; }

	void Decrypt(void* src, int a3, void* a4) JMP_THIS(0x632870);
	void Encrypt(void* src, int a3, void* a4) JMP_THIS(0x632740);

private:
	char Modulus[256];         // BigInt
	char Exponent[256];        // BigInt
	int BitPrecision;
};