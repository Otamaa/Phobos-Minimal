#pragma once

#include <ArrayClasses.h>
#include <CRT.h>

// vector of char* with builtin storage
class VectorNames
{
protected:
	DynamicVectorClass<const char*> Strings {};
	char* Buffer { nullptr };

public:
	VectorNames() = default;

	VectorNames(const char* pBuffer)
	{
		this->Tokenize(pBuffer);
	}

	~VectorNames()
	{
		this->Clear();
	}

	const char* operator[] (int index) const
	{
		return this->Strings.GetItemOrDefault(index);
	}

	COMPILETIMEEVAL const DynamicVectorClass<const char*>* Entries() const
	{
		return &this->Strings;
	}

	COMPILETIMEEVAL const char** ToString() const
	{
		return this->Strings.Items;
	}

	COMPILETIMEEVAL int Count() const
	{
		return this->Strings.Count;
	}

	void Clear()
	{
		if (this->Buffer)
		{
			this->Strings.Clear();
			YRMemory::Deallocate(this->Buffer);
			this->Buffer = nullptr;
		}
	}

	void Tokenize()
	{
		if (this->Buffer)
		{
			this->Strings.Reset();

			char* context = nullptr;
			for (auto cur = CRT::strtok(this->Buffer, ",", &context); cur && *cur; cur = CRT::strtok(nullptr, ",", &context))
			{
				this->Strings.AddItem(cur);
			}
		}
	}

	void Tokenize(const char* pBuffer)
	{
		if (pBuffer)
		{
			this->Clear();
			this->Buffer = CRT::strdup(pBuffer);
			this->Tokenize();
		}
	}
protected:
	VectorNames(const VectorNames& other) = delete;
	VectorNames& operator=(const VectorNames& other) = delete;
};
