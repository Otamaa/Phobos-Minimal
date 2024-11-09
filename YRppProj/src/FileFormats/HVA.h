#pragma once

// HVA file

class CCFileClass;
class Matrix3D;

class MotLib {
public:
	bool LoadedFailed;
	int LayerCount;
	int FrameCount;
	Matrix3D* Matrixes;

	MotLib() noexcept
		: LoadedFailed { false }
		, LayerCount { 0 }
		, FrameCount { 0 }
		, Matrixes { nullptr }

	{ }

	MotLib(CCFileClass* Source) noexcept
		: LoadedFailed { false }
		, LayerCount { 0 }
		, FrameCount { 0 }
		, Matrixes{ nullptr }
	{
		if (!this->ReadFile(Source)) {
			this->LoadedFailed = 1;
		}
	}

	~MotLib() noexcept {
		if(Matrixes) {
			Deallocate(Matrixes);
			Matrixes = nullptr;
		}
	}

	static bool IsInvalid(const MotLib* pThis)
		{ return !pThis || pThis->LoadedFailed; }

	// 0 for valid, non 0 for invalid
	bool ReadFile(CCFileClass* ccFile) const;// { JMP_THIS(0x5BD5C0); }
	void Scale(float scale) const;//{ JMP_THIS(0x5BD730); }
	void Clear() const;// { JMP_THIS(0x5BD7C0); }
};
