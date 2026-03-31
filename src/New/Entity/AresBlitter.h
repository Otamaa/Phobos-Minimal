
template<typename T>
class AresBlitter
{
public:

	virtual ~AresBlitter() = default;
	virtual void Blit_Copy(void* dst, void* src, int len, int zval, T* zbuf, T* abuf, int alvl, int warp) = 0;
	virtual void Blit_Copy_Tinted(void* dst, void* src, int len, int zval, T* zbuf, T* abuf, int alvl, int warp, T tint) = 0;
	virtual void Blit_Move(void* dst, void* src, int len, int zval, T* zbuf, T* abuf, int alvl, int warp) = 0;
	virtual void Blit_Move_Tinted(void* dst, void* src, int len, int zval, T* zbuf, T* abuf, int alvl, int warp, T tint) = 0;

};

template <typename T>
class AresPcxBlit final : public AresBlitter<T>
{
public:
	OPTIONALINLINE explicit AresPcxBlit(T mask, int imageWidth, int imageHeight, int cornerSize = 2) noexcept
		: Mask(mask), Width(imageWidth), Height(imageHeight), CornerSize(cornerSize) {}


	virtual ~AresPcxBlit() override final = default;

	virtual void Blit_Copy(void* dst, void* src, int len, int zval, T* zbuf, T* abuf, int alvl, int warp) override final
	{
		auto pDst = static_cast<T*>(dst);
		auto pSrc = static_cast<T*>(src);

		for (int i = 0; i < len; ++i)
		{
			int x = i;
			int y = zval;
			T pixel = *pSrc++;

			bool inTopLeft = (x < CornerSize && y < CornerSize);
			bool inTopRight = (x >= Width - CornerSize && y < CornerSize);
			bool inBottomLeft = (x < CornerSize && y >= Height - CornerSize);
			bool inBottomRight = (x >= Width - CornerSize && y >= Height - CornerSize);
			bool isCorner = inTopLeft || inTopRight || inBottomLeft || inBottomRight;

			if (pixel != Mask || !isCorner)
			{
				*pDst = pixel;
			}
			++pDst;
		}
	}

	virtual void Blit_Copy_Tinted(void* dst, void* src, int len, int zval, T* zbuf, T* abuf, int alvl, int warp, T tint)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

	virtual void Blit_Move(void* dst, void* src, int len, int zval, T* zbuf, T* abuf, int alvl, int warp)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

	virtual void Blit_Move_Tinted(void* dst, void* src, int len, int zval, T* zbuf, T* abuf, int alvl, int warp, T tint)
	{
		Blit_Copy(dst, src, len, zval, zbuf, abuf, alvl, 0);
	}

private:
	T Mask;
	int Width;
	int Height;
	int CornerSize;
};
