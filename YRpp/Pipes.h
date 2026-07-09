#pragma once

#include <CCFileClass.h>
#include <Base64.h>
#include <PKey.h>

class Pipe
{
public:
	explicit Pipe() = default;

	virtual ~Pipe()
	{
		if (this->ChainTo)
			this->ChainTo->ChainFrom = this->ChainFrom;

		if (this->ChainFrom)
			this->ChainFrom->Put_To(this->ChainTo);

		this->ChainFrom = nullptr;
		this->ChainTo = nullptr;
	}

	virtual int Flush()
	{
		if (this->ChainTo)
			return this->ChainTo->Flush();

		return 0;
	}

	virtual int End() { return(Flush()); }

	virtual void Put_To(Pipe* pPipe)
	{
		if (this->ChainTo != pPipe)
		{
			if (pPipe && pPipe->ChainFrom)
			{
				pPipe->ChainFrom->Put_To(nullptr);
				pPipe->ChainFrom = nullptr;
			}

			if (this->ChainTo)
			{
				this->ChainTo->ChainFrom = nullptr;
				this->ChainTo->Flush();
			}

			this->ChainTo = pPipe;
			if (this->ChainTo)
				this->ChainTo->ChainFrom = this;
		}
	}

	virtual int Put(void const* source, int length)
	{
		if (this->ChainTo)
			this->ChainTo->Put(source, length);

		return length;
	}

	virtual void Put_To(Pipe& pipe) { Put_To(&pipe); }

private:

	Pipe* ChainTo { nullptr };
	Pipe* ChainFrom { nullptr };

private:
	Pipe(Pipe& rvalue) = delete;
	Pipe& operator=(Pipe const& pipe) = delete;
};

class BufferPipe : public Pipe
{
public:
	explicit BufferPipe() = delete;
	explicit BufferPipe(void* pBuffer, int nLength) : Pipe {}, Buffer { pBuffer,nLength }
	{
	}

	virtual ~BufferPipe() override = default;

	virtual int Put(void const* pSource, int nLength) override final
	{
		if (this->Buffer.Buffer && pSource && nLength > 0)
		{
			if (this->Buffer.Size)
			{
				int nResidue = this->Buffer.Size - this->Index;
				if (nLength >= nResidue)
					nLength = nResidue;
			}
			if (nLength > 0)
				memcpy((char*)this->Buffer.Buffer + this->Index, pSource, nLength);

			this->Index += nLength;
			return nLength;
		}

		return 0;
	}

	MemoryBuffer Buffer;
	int Index;

private:
	BufferPipe(BufferPipe& rvalue) = delete;
	BufferPipe& operator=(BufferPipe const& pipe) = delete;
};

class LCWPipe : public Pipe
{
public:
	explicit LCWPipe() = delete;
	explicit LCWPipe(BOOL bControl, int nBlockSize) : Pipe {}
	{
		this->Control = bControl;
		this->SafetyMargin = nBlockSize / 0x80 + 1;
		if (this->SafetyMargin < 0x80)
			this->SafetyMargin = 0x80;
		this->Counter = 0;
		this->Buffer = nullptr;
		this->Buffer2 = nullptr;
		this->BlockSize = nBlockSize;
		this->Buffer = YRMemory::AllocateChecked(this->BlockSize + this->SafetyMargin);
		this->Buffer2 = YRMemory::AllocateChecked(this->BlockSize + this->SafetyMargin);
		this->BlockHeader_CompCount = -1;
		this->BlockHeader_UncompCount = 0;
	}

	virtual ~LCWPipe() override final
	{
		YRMemory::Deallocate(this->Buffer);
		YRMemory::Deallocate(this->Buffer2);
		this->Pipe::~Pipe();
	}

	static void __fastcall LCW_Comp(unsigned int source, unsigned int dest, unsigned int datasize)
		JMP_THIS(0x551E50);

	virtual int Flush() override final
	{
		JMP_THIS(0x5522D0);
	}

	virtual int Put(void const* pSource, int nLength) override final
	{
		JMP_THIS(0x5520A0);
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
	LCWPipe(LCWPipe& rvalue) = delete;
	LCWPipe& operator=(LCWPipe const& pipe) = delete;
};

class FilePipe : public Pipe
{
public:
	static OPTIONALINLINE COMPILETIMEEVAL DWORD vtable = 0x7E4DA0;

	virtual ~FilePipe()
	{
		if (this->File && this->HasOpened)
		{
			this->File->Close();
			this->HasOpened = 0;
			this->File = 0;
		}
		this->Pipe::~Pipe();
	}

	void Destroy() {

		if (this->File && this->HasOpened)
		{
			this->File->Close();
			this->HasOpened = 0;
			this->File = 0;
		}
		this->Pipe::~Pipe();
	}

	virtual int End() {
		const int retval = this->Flush();

		if (this->File && this->HasOpened ) {
				this->HasOpened = 0;
				this->File->Close();
		}

		return retval;
	}

	virtual int Put(void const* source, int length) {
		if(!this->File || !source || length <= 0)
			return 0;

		if(!this->File->IsOpen()) {
			this->HasOpened = true;
			this->File->Open1(FileAccessMode::Write);
		}

		return this->File->Write((void*)source, length);
	}

public:

	FileClass* File;
	bool HasOpened;
};

class Base64Pipe : public Pipe
{
public:
	enum class CodeControl {
		ENCODE,
		DECODE
	};

	Base64Pipe(CodeControl control) : Pipe(), Control(control), Counter(0), CBuffer(), PBuffer() {}
	virtual ~Base64Pipe() = default;

	virtual int Flush() override { 
		int len = 0;
		if(this->Counter) {
			int decoded = 0;
			char* pBuffer = nullptr;

			if (this->Control == CodeControl::ENCODE) {
				pBuffer = this->PBuffer;
				decoded = Base64::Encode(this->PBuffer, this->Counter, this->CBuffer, 4);
			} else {
				pBuffer = this->CBuffer;
				decoded = Base64::Decode(this->CBuffer, this->Counter, this->PBuffer, 3);
			}

			len = this->Pipe::Put(pBuffer, decoded);
			this->Counter = 0;
		}

		return len + this->Pipe::Flush();
	}

	virtual int Put(const void* source, int slen) override
	{
		if (!source || slen < 1) {
			return this->Pipe::Put(source, slen);
		}

		const char* pSource = static_cast<const char*>(source);
		size_t written = 0;

		const int inputBlock  = this->Control != CodeControl::ENCODE ? 4 : 3;
		const int outputBlock = this->Control != CodeControl::ENCODE ? 3 : 4;

		char* const inputBuffer  = this->Control != CodeControl::ENCODE ? this->CBuffer : this->PBuffer;
		char* const outputBuffer = this->Control != CodeControl::ENCODE ? this->PBuffer : this->CBuffer;

		auto ConvertBlock = [&](const void* src) -> int
		{
			return this->Control == CodeControl::ENCODE
				? Base64::Encode(src, inputBlock, outputBuffer, outputBlock)
				: Base64::Decode(src, inputBlock, outputBuffer, outputBlock);
		};

		// Finish a partially buffered block.
		if (this->Counter > 0) {
			int count = std::min(slen, inputBlock - this->Counter);

			std::memmove(
				inputBuffer + this->Counter,
				pSource,
				count
			);

			this->Counter += count;
			pSource += count;
			slen -= count;

			if (this->Counter == inputBlock) {
				written += this->Pipe::Put(
					outputBuffer,
					ConvertBlock(inputBuffer)
				);

				this->Counter = 0;
			}
		}

		// Process complete blocks directly.
		while (slen >= inputBlock) {
			written += this->Pipe::Put(
				outputBuffer,
				ConvertBlock(pSource)
			);

			pSource += inputBlock;
			slen -= inputBlock;
		}

		// Buffer any remaining bytes.
		if (slen > 0) {
			std::memmove(inputBuffer, pSource, slen);
			this->Counter = slen;
		}

		return static_cast<int>(written);
	}

private:
	CodeControl Control;
	int Counter;
	char CBuffer[4];
	char PBuffer[3];

private:
	Base64Pipe(const Base64Pipe& rvalue) = delete;
	Base64Pipe& operator=(const Base64Pipe&) = delete;
};

class BlowPipe : public Pipe
{
public:
	enum class CryptControl {
		ENCRYPT,
		DECRYPT
	};

public:
	BlowPipe(CryptControl control) : 
		BlowFish(nullptr), 
		CurrentBlock(),
		Counter(0),
		Control(control) {}

	virtual ~BlowPipe() JMP_THIS(0x632F90);

	virtual int Flush() override final {
		JMP_THIS(0x438060);
	}

	virtual int Put(void const* pSource, int nLength) override final {
		JMP_THIS(0x4380A0);
	}

	void Key(void* key, void* length) JMP_THIS(0x4381D0);

private:
  void* BlowFish;
  char CurrentBlock[8];
  int Counter;
  CryptControl Control;

private:
	BlowPipe(const BlowPipe& rvalue) = delete;
	BlowPipe& operator=(const BlowPipe&) = delete;
};

class Straw;
class PKPipe : public Pipe
{
public:
	virtual ~PKPipe() JMP_THIS(0x632FF0);

	virtual void Put_To(Pipe* pPipe) {
		JMP_THIS(0x632D10);
	}

	virtual int Put(void const* source, int length) {
		JMP_THIS(0x632DC0);
	}

	void Key(PKey* a2) JMP_THIS(0x632D60);
	PKey* Encrypted_Key_Length() JMP_THIS(0x632F30);
	PKey* Plain_Key_Length() JMP_THIS(0x632F60);

private:
  char IsGettingKey;
  Straw *Rand;
  BlowPipe BF;
  int Control;
  PKey* CipherKey;
  char Buffer[256];
  int Counter;
  int BytesLeft;

private:
	PKPipe(const PKPipe& rvalue) = delete;
	PKPipe& operator=(const PKPipe&) = delete;
};

class LZOPipe : public Pipe
{
public:
	enum class CodeControl {
		ENCODE,
		DECODE
	};

	LZOPipe(LZOPipe::CodeControl a2, int size) { JMP_THIS(0x55C2B0); }
	virtual ~LZOPipe() JMP_THIS(0x55C6D0);

	virtual int Put(void const* source, int length)
	{
		JMP_THIS(0x55C350);
	}
	
	int Flush() JMP_THIS(0x55C5E0);

private:
	LZOPipe::CodeControl Control;
	int Counter;
	char* Buffer1;
	char* Buffer2;
	int BlockSize;
	int MxBlockSize;
	short CompressedBytes;
	short UncompressedBytes;

private:
	LZOPipe(const LZOPipe& rvalue) = delete;
	LZOPipe& operator=(const LZOPipe&) = delete;
};