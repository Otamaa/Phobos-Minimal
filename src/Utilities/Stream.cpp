#include "Stream.h"
#include "Debug.h"

#include <SwizzleManagerClass.h>

#include <Objidl.h>

bool PhobosByteStream::ReadFromStream(IStream* pStm, const size_t Length)
{
	auto size = this->Data.size();
	this->Data.resize(size + Length);
	auto pv = reinterpret_cast<void*>(this->Data.data());

	ULONG out = 0;
	auto success = pStm->Read(pv, Length, &out);
	bool result(SUCCEEDED(success) && out == Length);

	if (!result)
		this->Data.resize(size);

	return result;
}

bool PhobosByteStream::WriteToStream(IStream* pStm) const
{
	const size_t Length(this->Data.size());
	auto pcv = reinterpret_cast<const void*>(this->Data.data());

	ULONG out = 0;
	auto success = pStm->Write(pcv, Length, &out);

	return SUCCEEDED(success) && out == Length;
}

bool PhobosByteStream::Read(data_t* Value, size_t Size)
{
	bool ret = false;

	if (this->Data.size() >= this->CurrentOffset + Size)
	{
		auto Position = &this->Data[this->CurrentOffset];
		std::memcpy(Value, Position, Size);
		ret = true;
	}

	this->CurrentOffset += Size;
	return ret;
}

void PhobosByteStream::Write(const data_t* Value, size_t Size)
{
	this->Data.insert(this->Data.end(), Value, Value + Size);
}

size_t PhobosByteStream::ReadBlockFromStream(IStream* pStm)
{
	ULONG out = 0;
	size_t Length = 0;

	if (SUCCEEDED(pStm->Read(&Length, sizeof(Length), &out)))
	{
		if (this->ReadFromStream(pStm, Length))
			return Length;
	}

	return 0;
}

bool PhobosByteStream::WriteBlockToStream(IStream* pStm) const
{
	ULONG out = 0;
	const size_t Length = this->Data.size();

	if (SUCCEEDED(pStm->Write(&Length, sizeof(Length), &out)))
		return this->WriteToStream(pStm);

	return false;
}

bool PhobosStreamReader::RegisterChange(void* newPtr)
{
	static_assert(sizeof(long) == sizeof(void*), "long and void* need to be of same size.");

	long oldPtr = 0;
	if (this->Load(oldPtr)) {
		if (SUCCEEDED(SwizzleManagerClass::Instance().Here_I_Am(oldPtr, newPtr)))
			return true;

		//GameDebugLog::Log("[PhobosStreamReader] Failed To RegisterChange for [%p] to [%p]", oldPtr, newPtr);
	}

	return false;
}
