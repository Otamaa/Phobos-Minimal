#include "DirStruct.h"

bool DirStruct::CompareToTwoDir(DirStruct& pBaseDir, DirStruct& pDirFrom)
{ return std::abs(pDirFrom.Raw) >= std::abs(this->Raw - pBaseDir.Raw); }

void DirStruct::Func_5B29C0(DirStruct& pDir2, DirStruct& pDir3)
{

	if (std::abs(pDir3.Raw) < std::abs(this->Raw - pDir2.Raw))
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

void DirStruct::SetDir(DirType dir)
{
	Raw = ((unsigned short)((unsigned char)dir * 256));
}

DirType DirStruct::GetDirFixed() const
{
	return (DirType)((((this->Raw / (32768 / 256)) + 1) / 2) & (int)DirType::Max);
}

DirType DirStruct::GetDir() const
{
	return (DirType)(this->Raw / 256);
}

void DirStruct::SetDir(size_t bit, size_t val)
{
	if (bit <= 16u)
		Raw = ((unsigned short)TranslateFixedPoint::Normal(bit, 16u, val));
}

size_t DirStruct::GetValue(size_t Bits, size_t offset)
{
	if (Bits <= 16u)
		return TranslateFixedPoint::Normal(16, Bits, this->Raw, offset);

	return 0;
}