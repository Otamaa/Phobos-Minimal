#include "ShapeTextPrinter.h"

constexpr inline char* SignSequence { "/%$,.!?|" };
constexpr inline int SignSequenceLength { std::char_traits<char>::length(SignSequence) };

int ShapeTextPrinter::GetSignIndex(const char sign)
{
	return (std::find(SignSequence, SignSequence + SignSequenceLength, sign) - SignSequence);

}

void ShapeTextPrinter::BuildFrames(std::vector<int>& vFrames , const char* const text , const int baseNumberFrame , const int baseSignFrame)
{
	for (size_t i = 0; i < strlen(text); i++)
	{
		int frame = 0;

		if (isdigit(text[i]))
		{
			frame = baseNumberFrame + text[i] - '0';
		}
		else
		{
			int signIndex = GetSignIndex(text[i]);

			if (signIndex < SignSequenceLength)
				frame = baseSignFrame + signIndex;
			else
				return;
		}

		vFrames.push_back(frame);
	}
}

void ShapeTextPrinter::PrintShape
(
	const char* text,
	const ShapeTextPrintData& data,
	Point2D* pPosDraw,
	RectangleStruct* pRBound,
	DSurface* pSurface,
	Point2D offset, // ?
	BlitterFlags eBlitterFlags,
	int iBright,
	int iTintColor
)
{
	std::vector<int> vFrames;
	ShapeTextPrinter::BuildFrames(vFrames, text, data.BaseNumberFrame, data.BaseSignFrame);

	for (const int& frame : vFrames)
	{
		pSurface->DrawSHP
		(
			const_cast<ConvertClass*>(data.Palette),
			const_cast<SHPStruct*>(data.Shape),
			frame,
			pPosDraw,
			pRBound,
			BlitterFlags::None,
			0,
			0,
			ZGradient::Ground,
			1000,
			0,
			nullptr,
			0,
			0,
			0
		);

		pPosDraw->X += data.Interval.X;
		pPosDraw->Y += data.Interval.Y;
	}
}
