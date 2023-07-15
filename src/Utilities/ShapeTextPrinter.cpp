#include "ShapeTextPrinter.h"

constexpr inline char* SignSequence { "/%$,.!?|" };
constexpr inline int SignSequenceLength { std::char_traits<char>::length(SignSequence) };

int ShapeTextPrinter::GetSignIndex(const char sign)
{
	return (std::find(SignSequence, SignSequence + SignSequenceLength, sign) - SignSequence);
}

std::vector<int> ShapeTextPrinter::BuildFrames(const std::string& text, const int baseNumberFrame , const int baseSignFrame)
{
	std::vector<int> vFrames;

	for (size_t i = 0; i < text.size(); i++)
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
				break;
		}

		vFrames.emplace_back(frame);
	}

	return vFrames;
}

void ShapeTextPrinter::PrintShape
(
	const std::string& text,
	ShapeTextPrintData data,
	Point2D* pPosDraw,
	RectangleStruct* pRBound,
	DSurface* pSurface,
	Point2D offset, // ?
	BlitterFlags eBlitterFlags,
	int iBright,
	int iTintColor
)
{
	if (text.empty())
		return;

	std::vector<int> frames = ShapeTextPrinter::BuildFrames(text, data.BaseNumberFrame, data.BaseExtraFrame);

	for (int frame : frames)
	{
		pSurface->DrawSHP
		(
			data.Palette,
			data.Shape,
			frame,
			pPosDraw,
			pRBound,
			eBlitterFlags,
			0,
			0,
			ZGradient::Ground,
			iBright,
			iTintColor,
			nullptr,
			0,
			0,
			0
		);

		pPosDraw->X += data.Spacing.X;
		pPosDraw->Y -= data.Spacing.Y;
	}
}
