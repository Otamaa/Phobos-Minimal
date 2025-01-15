#include "ShapeTextPrinter.h"

#include <Surface.h>

COMPILETIMEEVAL OPTIONALINLINE char* SignSequence { "/%$,.!?|" };
COMPILETIMEEVAL OPTIONALINLINE size_t SignSequenceLength { std::char_traits<char>::length(SignSequence) };

size_t ShapeTextPrinter::GetSignIndex(const char sign)
{
	char* end = SignSequence + SignSequenceLength;
	char* iter = std::find(SignSequence, end , sign);
	return iter != end ? std::distance(SignSequence, iter) : -1;
}

std::vector<int> ShapeTextPrinter::BuildFrames(const std::string& text, const int baseNumberFrame , const int baseSignFrame)
{
	std::vector<int> vFrames;

	for (const auto& item : text) {

		int frame = 0;

		if (isdigit(item)) {
			frame = baseNumberFrame + item - '0';
		} else {
			const size_t signIndex = GetSignIndex(item);

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
