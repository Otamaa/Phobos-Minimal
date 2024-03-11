#pragma once
#include <Utilities/TemplateDef.h>

// only numbers and sign
class ShapeTextPrintData
{
public:
	// Shape
	SHPStruct* Shape { nullptr };
	ConvertClass* Palette { nullptr };
	int BaseNumberFrame { 0 };	// frame index of 0
	int BaseExtraFrame { 0 };		// as sequence ShapeTextPrinter::SignSequence
	Point2D Spacing {};

	ShapeTextPrintData(SHPStruct* shape, ConvertClass* palette, int iBaseNumberFrame, int baseExtraFrame, const Point2D& spacing)
		: Shape { shape }
		, Palette { palette }
		, BaseNumberFrame { iBaseNumberFrame }
		, BaseExtraFrame { baseExtraFrame }
		, Spacing { spacing }
	{ }

	~ShapeTextPrintData() = default;
	ShapeTextPrintData() = default;
	ShapeTextPrintData(const ShapeTextPrintData&) = default;
	ShapeTextPrintData(ShapeTextPrintData&&) = default;
	ShapeTextPrintData& operator=(const ShapeTextPrintData& other) = default;
};

class ShapeTextPrinter
{
private:
	static size_t GetSignIndex(const char sign);
	static std::vector<int> BuildFrames(const std::string& text, const int baseNumberFrame, const int baseSignFrame);

public:

	static void PrintShape
	(
		const std::string& text,
		ShapeTextPrintData data,
		Point2D* pPosDraw,
		RectangleStruct* pRBound,
		DSurface* pSurface,
		Point2D offset = Point2D::Empty,
		BlitterFlags eBlitterFlags = BlitterFlags::None,
		int iBright = 1000,
		int iTintColor = 0
	);
};
