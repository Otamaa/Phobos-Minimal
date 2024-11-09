#pragma once

struct TranslateFixedPoint
{
	static inline size_t Normal(size_t bitsFrom, size_t bitsTo, size_t value, size_t offset = 0)
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

	static inline constexpr size_t CompileTime(size_t bitsFrom, size_t bitsTo, size_t value, size_t offset = 0)
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

	template<size_t BitsFrom, size_t BitsTo>
	static inline constexpr size_t TemplatedCompileTime(size_t value, size_t offset = 0)
	{
		constexpr size_t MaskIn = ((1u << BitsFrom) - 1);
		constexpr size_t MaskOut = ((1u << BitsTo) - 1);

		if constexpr (BitsFrom > BitsTo)
			return (((((value & MaskIn) >> (BitsFrom - BitsTo - 1)) + 1) >> 1) + offset) & MaskOut;
		else if constexpr (BitsFrom < BitsTo)
			return (((value - offset) & MaskIn) << (BitsTo - BitsFrom)) & MaskOut;
		else
			return value & MaskOut;
	}

	template<size_t BitsFrom, size_t BitsTo ,size_t Value>
	static inline constexpr size_t TemplatedCompileTime(size_t offset = 0)
	{
		constexpr size_t MaskIn = ((1u << BitsFrom) - 1);
		constexpr size_t MaskOut = ((1u << BitsTo) - 1);

		if constexpr (BitsFrom > BitsTo)
			return (((((Value & MaskIn) >> (BitsFrom - BitsTo - 1)) + 1) >> 1) + offset) & MaskOut;
		else if constexpr (BitsFrom < BitsTo)
			return (((Value - offset) & MaskIn) << (BitsTo - BitsFrom)) & MaskOut;
		else
			return Value & MaskOut;
	}
};