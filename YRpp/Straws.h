#pragma once

#include <YRPP.h>

class Straw
{
public:
	static OPTIONALINLINE COMPILETIMEEVAL DWORD vtable = 0x7E61F0;

	COMPILETIMEEVAL explicit Straw() = default;

	virtual ~Straw()
	{
		if (this->ChainTo)
			ChainTo->ChainFrom = this->ChainFrom;

		if (this->ChainFrom)
			ChainFrom->Get_From(ChainTo);

		this->ChainFrom = nullptr;
		this->ChainTo = nullptr;
	}

	virtual void Get_From(Straw* pStraw)
	{
		if (this->ChainTo != pStraw)
		{
			if (pStraw && pStraw->ChainFrom)
			{
				pStraw->ChainFrom->Get_From(nullptr);
				pStraw->ChainFrom = nullptr;
			}

			if (this->ChainTo)
				this->ChainTo->ChainFrom = nullptr;

			this->ChainTo = pStraw;
			if (this->ChainTo)
				this->ChainTo->ChainFrom = this;
		}
	}

	virtual int Get(void* pBuffer, int slen)
	{
		if (this->ChainTo)
			return this->ChainTo->Get(pBuffer, slen);

		return 0;
	}

	COMPILETIMEEVAL FORCEDINLINE void Get_From(Straw& pipe) { Get_From(&pipe); }

public:

	Straw* ChainTo { nullptr };
	Straw* ChainFrom { nullptr };

private:
	Straw(Straw& rvalue) = delete;
	Straw& operator=(Straw const& pipe) = delete;
};

class BufferStraw : public Straw
{
public:
	COMPILETIMEEVAL explicit BufferStraw() = delete;
	explicit BufferStraw(void* pBuffer, int nLength) : Straw {}, Buffer { pBuffer,nLength }
	{ }

	virtual ~BufferStraw() override final {
	}

	virtual int Get(void* pBuffer, int slen) override final
	{
		if (this->Buffer.Buffer && pBuffer && slen > 0)
		{
			if (this->Buffer.Size)
			{
				int nResidue = this->Buffer.Size - this->Index;
				if (slen > nResidue)
					slen = nResidue;
			}

			if (slen > 0)
				CRT::memcpy(pBuffer, (char*)this->Buffer.Buffer + this->Index, slen);

			this->Index += slen;
			return slen;
		}
		return 0;
	}

public:

	MemoryBuffer Buffer;
	int Index {0};

private:
	BufferStraw(BufferStraw& rvalue) = delete;
	BufferStraw& operator=(BufferStraw const& pipe) = delete;
};

class LCWStraw : public Straw
{
public:
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7ECF44l;

	COMPILETIMEEVAL explicit LCWStraw() = delete;
	explicit LCWStraw(BOOL bControl, size_t nBlockSize) : Straw {}
	{
		this->Control = bControl;
		this->SafetyMargin = nBlockSize / 0x80 + 1;
		this->Counter = 0;
		this->Buffer = nullptr;
		this->Buffer2 = nullptr;
		this->BlockSize = nBlockSize;
		this->Buffer = YRMemory::AllocateChecked(this->BlockSize + this->SafetyMargin);
		if (!this->Control)
			this->Buffer2 = YRMemory::AllocateChecked(this->BlockSize + this->SafetyMargin);
	}

	virtual ~LCWStraw() override final {
		YRMemory::Deallocate(this->Buffer);
		if (this->Buffer2)
			YRMemory::Deallocate(this->Buffer2);
	}

	virtual int Get(void* pBuffer, int slen) override final {
		JMP_THIS(0x552490);
	}

public:
	BOOL Control;
	int Counter;
	void* Buffer;
	void* Buffer2;
	size_t BlockSize;
	int SafetyMargin;
	short BlockHeader_CompCount;
	short BlockHeader_UncompCount;

private:
	LCWStraw(LCWStraw& rvalue) = delete;
	LCWStraw& operator=(LCWStraw const& pipe) = delete;
};

class FileStraw :public Straw
{
public:

	static OPTIONALINLINE COMPILETIMEEVAL DWORD vtable = 0x7E4D90;

	virtual ~FileStraw() override final {
		if (this->File && this->HasOpened)
		{
			this->File->Close();
			this->HasOpened = 0;
			this->File = 0;
		}
		this->Straw::~Straw();
	}

	virtual int Get(void* pBuffer, int slen) override final {
		JMP_THIS(0x7BA530);
	}

	void Destroy() {
		JMP_THIS(0x7BA590);
	}

	FileClass* File;
	bool HasOpened;
};
static_assert(sizeof(FileStraw) == 0x14);