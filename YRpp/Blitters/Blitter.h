#pragma once

#include <YRPPCore.h>

#include <Drawing.h>
#include <AlphaLightingRemapClass.h>

// All of those blitters can be found at 48EBF0!

// All Westwood fucking jesus blitters goes here!
class Blitter
{
public :
	static COMPILETIMEEVAL reference<Blitter, 0x7F7C0C> const Blit_Trans_Uchar_vftable {};
	static COMPILETIMEEVAL reference<Blitter, 0x7F7BF4> const Blit_Trans_Ushort_vftable {};
public:
	virtual ~Blitter() = default;
	virtual void Blit_Copy(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp) = 0;
	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) = 0;
	virtual void Blit_Move(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp) = 0;
	virtual void Blit_Move_Tinted(void* dst, byte* src, int len, int zval, WORD* zbuf, WORD* abuf, int alvl, int warp, WORD tint) = 0;

protected:
	OPTIONALINLINE static WORD* Lookup_Alpha_Remapper(int alvl, AlphaLightingRemapClass* remapper)
	{
		// convert alvl from [0, 2000] into [0, 254]
		int level = MinImpl(254, 261* MaxImpl(0, alvl) >> 11);
		return remapper->Table[level];
	}
};

// And those are compressed one :(
class RLEBlitter
{
public:
	virtual ~RLEBlitter() = default;
	virtual void Blit_Copy(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust) = 0;
	virtual void Blit_Copy_Tinted(void* dst, byte* src, int len, int line, int zbase, WORD* zbuf, WORD* abuf, int alvl, int warp, byte* zadjust, WORD tint) = 0;

protected:
	OPTIONALINLINE static WORD* Lookup_Alpha_Remapper(int alvl, AlphaLightingRemapClass* remapper)
	{
		// convert alvl from [0, 2000] into [0, 254]
		int level = MinImpl(254, 261* MaxImpl(0, alvl) >> 11);
		return remapper->Table[level];
	}

	template<bool UseZBuffer, bool UseABuffer, typename T>
	OPTIONALINLINE static void Process_Pre_Lines(T*& dest, byte*& src, int& len, const int& line, WORD* zbuf, WORD* abuf)
	{
		if (line > 0)
		{
			int off = -line;
			do
			{
				if (*src++)
					++off;
				else
					off += *src++;
			}
			while (off < 0);

			dest += off;
			len -= off;

			if COMPILETIMEEVAL (UseZBuffer)
			{
				zbuf += off;
				ZBuffer::Instance->AdjustPointer(zbuf);
			}
			if COMPILETIMEEVAL (UseABuffer)
			{
				abuf += off;
				ABuffer::Instance->AdjustPointer(abuf);
			}
		}
	}

	template<bool UseZBuffer, bool UseABuffer, typename T, typename Fn>
	OPTIONALINLINE static void Process_Pixel_Datas(T* dest, byte* src, int len, int zbase, WORD* zbuf, WORD* abuf, byte* zadjust, Fn f)
	{
		if (len < 0)
			return;

		while (len > 0)
		{
			if (byte srcv = *src++)
			{
				if COMPILETIMEEVAL (UseZBuffer && UseABuffer)
					f(*dest, srcv, zbase, *zbuf++, *zadjust++, *abuf++);
				else if COMPILETIMEEVAL (UseZBuffer && !UseABuffer)
					f(*dest, srcv, zbase, *zbuf++, *zadjust++);
				else if COMPILETIMEEVAL (!UseZBuffer && UseABuffer)
					f(*dest, srcv, *abuf++);
				else // !UseZBuffer && !UseABuffer
					f(*dest, srcv);

				++dest;
				--len;
			}
			else
			{
				byte off = *src++;
				len -= off;
				dest += off;

				if COMPILETIMEEVAL (UseZBuffer)
				{
					zbuf += off;
					zadjust += off;
				}
				if COMPILETIMEEVAL (UseABuffer)
					abuf += off;
			}

			if COMPILETIMEEVAL (UseZBuffer)
				ZBuffer::Instance->AdjustPointer(zbuf);
			if COMPILETIMEEVAL (UseABuffer)
				ABuffer::Instance->AdjustPointer(abuf);
		}
	}
};

#define DEFINE_BLITTER(x) \
template<typename T> \
class x final : public Blitter

#define DEFINE_RLE_BLITTER(x) \
template<typename T> \
class x final : public RLEBlitter
