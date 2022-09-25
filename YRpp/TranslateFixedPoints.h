#pragma once

inline size_t TranslateFixedPointNoconstexpr(size_t bitsFrom, size_t bitsTo, unsigned int value, unsigned int offset = 0)
{
	const size_t MaskIn = ((1u << bitsFrom) - 1);
	const size_t MaskOut = ((1u << bitsTo) - 1);

	if (bitsFrom > bitsTo)
	{
		// converting down
		return (((((value & MaskIn) >> (bitsFrom - bitsTo - 1)) + 1) >> 1) + offset) & MaskOut;

	}
	else if (bitsFrom < bitsTo)
	{
		// converting up
		return (((value - offset) & MaskIn) << (bitsTo - bitsFrom)) & MaskOut;

	}
	else
	{
		return value & MaskOut;
	}
}
