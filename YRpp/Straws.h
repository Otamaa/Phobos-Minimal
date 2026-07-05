#pragma once

#include <CRT.h>
#include <CCFileClass.h>

class PKey;
class Random3Straw;

class Straw
{
public:
	static OPTIONALINLINE COMPILETIMEEVAL DWORD vtable = 0x7E61F0;

	COMPILETIMEEVAL explicit Straw() = default;

	virtual ~Straw()
	{
		if (this->ChainTo)
			this->ChainTo->ChainFrom = this->ChainFrom;

		if (this->ChainFrom)
			this->ChainFrom->Straw::Get_From(this->ChainTo);

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

	static size_t __fastcall Read_Line(Straw* file, char* buffer, int len, bool* eof)
	{
		int count; // ebp
		char* source; // esi
		char c; // [esp+Bh] [ebp-5h] BYREF
		int v11; // [esp+Ch] [ebp-4h] MAPDST

		if (!len || !buffer)
		{
			return 0;
		}
		count = 0;
		if (file->Get(&c, 1) == 1)
		{
			source = buffer;
			v11 = 1 - (DWORD)buffer;
			v11 = 1 - (DWORD)buffer;
			while (c != '\n')
			{
				if (c != '\r' && (int)&source[v11] < len)
				{
					*source = c;
					++count;
					++source;
				}
				if (file->Get(&c, 1) != 1)
				{
					goto LABEL_10;
				}
			}
		}
		else
		{
		LABEL_10:
			*eof = 1;
			buffer[count] = 0;
		}
		buffer[count] = 0;
		return strlen(buffer);
	}
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

	explicit BufferStraw(void* pBuffer, int nLength) 
		: Straw {}, Buffer { pBuffer,nLength }
	{ }

	virtual ~BufferStraw() override final {
		this->Buffer.~MemoryBuffer();
		this->Straw::~Straw();
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

	explicit LCWStraw() = delete;

	explicit LCWStraw(BOOL bControl, size_t nBlockSize) : Straw(),
		Control(bControl),
		Counter(),
		Buffer(nullptr),
		Buffer2(nullptr),
		BlockSize(nBlockSize),
		SafetyMargin(nBlockSize / 0x80 + 1),
		BlockHeader_CompCount(),
		BlockHeader_UncompCount()
	{
		const auto safety = nBlockSize / 0x80 + 1;
		this->Buffer = YRMemory::AllocateChecked(nBlockSize + safety);
		if (!bControl)
			this->Buffer2 = YRMemory::AllocateChecked(nBlockSize + safety);
	}

	virtual ~LCWStraw() override final {
		YRMemory::Deallocate(this->Buffer);
		YRMemory::Deallocate(this->Buffer2);

		this->Straw::~Straw();
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

	FileStraw(FileClass* pFile) : 
		Straw(), File(pFile), HasOpened(0)
	{}

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

class CacheStraw : public Straw
{
public:
	CacheStraw(const MemoryBuffer& buffer) : Straw(), BufferPtr(buffer), Index(0), Length(0) {}
	CacheStraw(int length = 4096) : Straw(), BufferPtr(length), Index(0), Length(0) {}

	virtual ~CacheStraw() {
		this->BufferPtr.~MemoryBuffer();
		this->Straw::~Straw();
	}

	virtual int Get(void* source, int slen) override
	{
		if (!this->BufferPtr.Is_Valid() || !source || slen <= 0) {
        	return 0;
   		}

		auto* pDest = static_cast<char*>(source);
		int remaining = slen;
		int totalRead = 0;

		while (remaining > 0) {
			if (this->Length > 0) {
				const int count = std::min(this->Length, remaining);

				memcpy(
					pDest,
					this->BufferPtr.operator char*() + this->Index,
					count
				);

				this->Index += count;
				this->Length -= count;

				pDest += count;
				remaining -= count;
				totalRead += count;
			}

			if (remaining == 0) {
				break;
			}

			this->Length = Straw::Get(
				this->BufferPtr.Get_Buffer(),
				this->BufferPtr.Size
			);
			this->Index = 0;

			if (this->Length == 0) {
				break;
			}
		}

    	return totalRead;
	}

private:
	bool Is_Valid() { return BufferPtr.Is_Valid(); }

private:
	MemoryBuffer BufferPtr;
	int Index;
	int Length;

private:
	CacheStraw(const CacheStraw&) = delete;
	CacheStraw& operator=(const CacheStraw&) = delete;
};

class Base64Straw : public Straw
{
public:
	enum class CodeControl {
		ENCODE,
		DECODE
	};

public:
    Base64Straw(CodeControl control) :
		Straw(),
		Control(control),
		Counter(0),
		CBuffer(), 
		PBuffer()
	{}

	virtual ~Base64Straw() = default;

    virtual int Get(void* source, int slen) override { JMP_THIS(0x42DF90); }

private:
    CodeControl Control;
    int Counter;
    char CBuffer[4];
    char PBuffer[3];

private:
    Base64Straw(const Base64Straw&) = delete;
    Base64Straw& operator=(const Base64Straw&) = delete;
};

class BlowStraw : public Straw
{
public:

	enum class CryptControl {
		ENCRYPT,
		DECRYPT
	};

public:
  BlowStraw(CryptControl control) 
	  : Straw() , BF(nullptr), Buffer(), Counter(0), Control(control) {}

  virtual ~BlowStraw() override { 
		if(this->BF) {
			ReleaseBFObj(this->BF);
			YRMemory::free(this->BF);
		}

		this->BF = nullptr;
		this->Straw::~Straw();

    }

  virtual int Get(void* source, int slen) override { JMP_THIS(0x438210); }

  void Key(void* source, int slen){ JMP_THIS(0x438300); }
protected:
	void __fastcall ReleaseBFObj(void* pBF){
		JMP_THIS(0x437FC0);
	}

private:
  void* BF; //Blowfish smart pointer for interface i presume
  char Buffer[8];
  int Counter;
  CryptControl Control;

private:
    BlowStraw(const BlowStraw&) = delete;
    BlowStraw& operator=(const BlowStraw&) = delete;
};

class Random3Straw;
class PKStraw : public Straw
{
public:
	enum class CodeControl {
		ENCODE,
		DECODE
	};

  PKStraw(PKStraw::CodeControl control , Random3Straw* rand ) :
	 Straw(),
  	 IsGettingKey(1),
	 Rand(rand),
	 BF((BlowStraw::CryptControl)control),
	 Control(control),
	 CipherKey(nullptr),
	 Buffer(),
	 Counter(0),
	 BytesLeft(0) {
	this->PKStraw::Get_From(&this->BF);
  }

  virtual ~PKStraw() override {
		this->BF.~BlowStraw();
		this->Straw::~Straw();
  }

  virtual int Get(void* source, int slen) override { JMP_THIS(0x633130); }

  void Key(PKey* key){ JMP_THIS(0x633110); }

  void Get_From(Straw *straw) JMP_THIS(0x6330C0);
  int Plain_Key_Length() JMP_THIS(0x633330);
  int Encrypted_Key_Length() JMP_THIS(0x633300);
  BlowStraw* GetBF() { return &this->BF; }

private:
  char IsGettingKey;
  Random3Straw* Rand;
  BlowStraw BF;
  CodeControl Control;
  void *CipherKey;
  char Buffer[256];
  int Counter;
  int BytesLeft;

private:
    PKStraw(const PKStraw&) = delete;
    PKStraw& operator=(const PKStraw&) = delete;
};