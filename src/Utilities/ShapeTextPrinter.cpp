#include "ShapeTextPrinter.h"

#include <Surface.h>

COMPILETIMEEVAL OPTIONALINLINE char SignSequence[] { "/%$,.!?|" };
COMPILETIMEEVAL OPTIONALINLINE size_t SignSequenceLength { std::char_traits<char>::length(SignSequence) };

size_t ShapeTextPrinter::GetSignIndex(const char sign)
{
	const char* end = SignSequence + SignSequenceLength;
	const char* iter = std::find(SignSequence, end , sign);
	return iter != end ? std::distance(SignSequence, iter) : -1;
}

void ShapeTextPrinter::BuildFrames(std::vector<int>& result, const std::string& text, const int baseNumberFrame , const int baseSignFrame)
{
	result.reserve(text.size());

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

		result.emplace_back(frame);
	}
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

	std::vector<int> frames {};
	ShapeTextPrinter::BuildFrames(frames , text, data.BaseNumberFrame, data.BaseExtraFrame);

	for (int& frame : frames)
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
