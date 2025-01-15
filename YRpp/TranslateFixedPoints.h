#pragma once

struct TranslateFixedPoint
{
	static OPTIONALINLINE size_t Normal(size_t bitsFrom, size_t bitsTo, size_t value, size_t offset = 0)
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

	static OPTIONALINLINE COMPILETIMEEVAL size_t CompileTime(size_t bitsFrom, size_t bitsTo, size_t value, size_t offset = 0)
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
	static OPTIONALINLINE COMPILETIMEEVAL size_t TemplatedCompileTime(size_t value, size_t offset = 0)
	{
		COMPILETIMEEVAL size_t MaskIn = ((1u << BitsFrom) - 1);
		COMPILETIMEEVAL size_t MaskOut = ((1u << BitsTo) - 1);

		if COMPILETIMEEVAL (BitsFrom > BitsTo)
			return (((((value & MaskIn) >> (BitsFrom - BitsTo - 1)) + 1) >> 1) + offset) & MaskOut;
		else if COMPILETIMEEVAL (BitsFrom < BitsTo)
			return (((value - offset) & MaskIn) << (BitsTo - BitsFrom)) & MaskOut;
		else
			return value & MaskOut;
	}

	template<size_t BitsFrom, size_t BitsTo ,size_t Value>
	static OPTIONALINLINE COMPILETIMEEVAL size_t TemplatedCompileTime(size_t offset = 0)
	{
		COMPILETIMEEVAL size_t MaskIn = ((1u << BitsFrom) - 1);
		COMPILETIMEEVAL size_t MaskOut = ((1u << BitsTo) - 1);

		if COMPILETIMEEVAL (BitsFrom > BitsTo)
			return (((((Value & MaskIn) >> (BitsFrom - BitsTo - 1)) + 1) >> 1) + offset) & MaskOut;
		else if COMPILETIMEEVAL (BitsFrom < BitsTo)
			return (((Value - offset) & MaskIn) << (BitsTo - BitsFrom)) & MaskOut;
		else
			return Value & MaskOut;
	}
};