#include <Phobos.h>

class Checksummer
{
	static const size_t Size = 4;
public:
	static constexpr reference<unsigned int, 0x81F7B4, 256> const Table {};

	Checksummer() : Value(0), ByteIndex(0)
	{
		for (int i = 0; i < Size; ++i)
		{
			this->Bytes[i] = 0;
		}
	}

	DWORD GetValue() const
	{
		return this->GetValueInline();
	}

	DWORD Intermediate() const
	{
		return this->Value;
	}

	void Commit()
	{
		this->CommitInline();
	}

	void Add(const void* data, size_t c_bytes)
	{
		if (data && c_bytes)
		{
			auto bytes = reinterpret_cast<const BYTE*>(data);

			// fill the current block
			while (this->ByteIndex != 0 && this->ByteIndex < Size && c_bytes)
			{
				this->AddInline(*bytes++);
				--c_bytes;
			}

			// take the full blocks
			const auto blocks = c_bytes / Size;
			for (auto i = 0u; i < blocks; ++i)
			{
				this->Value = Process(bytes, Size, this->Value);
				bytes += Size;
			}
			c_bytes -= blocks * Size;

			// fill in the remainder
			while (c_bytes--)
			{
				this->AddInline(*bytes++);
			}
		}
	}

	void Add(BYTE value)
	{
		this->AddInline(value);
	}

	void Add(bool value)
	{
		this->Add(static_cast<BYTE>(value != 0));
	}

	void Add(char value)
	{
		this->Add(static_cast<BYTE>(value));
	}

	void Add(signed char value)
	{
		this->Add(static_cast<BYTE>(value));
	}

	void Add(const char* string)
	{
		if (string)
		{
			this->Add(string, strlen(string));
		}
	}

	template <typename T>
	void Add(const T& value)
	{
		this->Add(&value, sizeof(T));
	}

	static DWORD Process(const void* data, size_t size, DWORD initial)
	{
		auto bytes = reinterpret_cast<const BYTE*>(data);
		auto ret = ~initial;

		for (auto i = 0u; i < size; ++i)
		{
			ret = Table[*bytes++ ^ static_cast<BYTE>(ret)] ^ (ret >> 8);
		}

		return ~ret;
	}

	static DWORD Process(const char* string, DWORD initial)
	{
		auto bytes = reinterpret_cast<const BYTE*>(string);
		auto ret = ~initial;

		while (*bytes)
		{
			ret = Table[*bytes++ ^ static_cast<BYTE>(ret)] ^ (ret >> 8);
		}

		return ~ret;
	}

protected:
	void Fill() const
	{
		// this check is missing in the original, which makes it
		// write beyond the array, ie. outside the class' memory.
		if (this->ByteIndex < Size)
		{
			this->Bytes[this->ByteIndex] = static_cast<BYTE>(this->ByteIndex);
			for (auto i = this->ByteIndex + 1; i < Size; ++i)
			{
				this->Bytes[i] = this->Bytes[0];
			}
		}
	}

	FORCEINLINE void AddInline(BYTE value)
	{
		// clear old data
		if (this->ByteIndex == 0)
		{
			for (int i = 0; i < Size; ++i)
			{
				this->Bytes[i] = 0;
			}
		}

		this->Bytes[this->ByteIndex++] = value;

		if (this->ByteIndex == Size)
		{
			this->CommitInline();
		}
	}

	FORCEINLINE DWORD GetValueInline() const
	{
		// nothing to check
		if (!this->ByteIndex)
		{
			return this->Value;
		}

		// fill the remaining bytes
		this->Fill();

		// project the value without changing internal state
		return Process(this->Bytes, Size, this->Value);
	}

	FORCEINLINE void CommitInline()
	{
		this->Value = this->GetValueInline();
		this->ByteIndex = 0;
	}

	DWORD Value;
	size_t ByteIndex;
	mutable BYTE Bytes[Size];
};

DEFINE_OVERRIDE_HOOK(0x4A1C10, Checksummer_Add_BYTE, 5)
{
	GET(Checksummer*, pThis, ECX);
	REF_STACK(const BYTE, value, STACK_OFFS(0x0, -0x4));

	pThis->Add(value);

	return 0x4A1C8E;
}

DEFINE_OVERRIDE_HOOK(0x4A1CA0, Checksummer_Add_bool, 5)
{
	GET(Checksummer*, pThis, ECX);
	REF_STACK(const bool, value, STACK_OFFS(0x0, -0x4));

	pThis->Add(value);

	return 0x4A1D23;
}

DEFINE_OVERRIDE_HOOK(0x4A1D30, Checksummer_Add_WORD, 5)
{
	GET(Checksummer*, pThis, ECX);
	REF_STACK(const WORD, value, STACK_OFFS(0x0, -0x4));

	pThis->Add(value);

	return 0x4A1D46;
}

DEFINE_OVERRIDE_HOOK(0x4A1D50, Checksummer_Add_DWORD, 8)
{
	GET(Checksummer*, pThis, ECX);
	REF_STACK(const DWORD, value, STACK_OFFS(0x0, -0x4));

	pThis->Add(value);

	return 0x4A1D64;
}

DEFINE_OVERRIDE_HOOK(0x4A1D70, Checksummer_Add_float, 8)
{
	GET(Checksummer*, pThis, ECX);
	REF_STACK(const float, value, STACK_OFFS(0x0, -0x4));

	pThis->Add(value);

	return 0x4A1D84;
}

DEFINE_OVERRIDE_HOOK(0x4A1D90, Checksummer_Add_double, 8)
{
	GET(Checksummer*, pThis, ECX);
	REF_STACK(const double, value, STACK_OFFS(0x0, -0x4));

	pThis->Add(value);

	return 0x4A1DAC;
}

DEFINE_OVERRIDE_HOOK(0x4A1DE0, Checksummer_Add_Buffer, 6)
{
	GET(Checksummer*, pThis, ECX);
	GET_STACK(const void*, data, STACK_OFFS(0x0, -0x4));
	GET_STACK(size_t, length, STACK_OFFS(0x0, -0x8));

	pThis->Add(data, length);

	R->EAX(pThis->GetValue());
	return 0x4A1FA6;
}