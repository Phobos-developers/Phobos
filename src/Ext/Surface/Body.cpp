#include "Body.h"

#include <Drawing.h>

void SurfaceExt::BlurRect(const RectangleStruct& rect, int blurSize)
{
	if (blurSize <= 1)
		return;
	if (this->GetBytesPerPixel() < 2)
		return;
	if (rect.Width <= 0 || rect.Height <= 0)
		return;

	const auto bound = Drawing::Intersect(rect, this->GetRect());
	if (bound.Width <= 0 || bound.Height <= 0)
		return;

	if (auto ptr = (WORD*)this->Lock(bound.X, bound.Y))
	{
		for (int yy = bound.Y; yy < bound.Height; ++yy)
		{
			const int line = yy * (this->GetPitch() / sizeof(WORD));
			auto p = &ptr[line];
			for (int xx = bound.X; xx < bound.Width; ++xx)
			{
				int avgR = 0, avgG = 0, avgB = 0;

				int r = std::min(xx + blurSize, bound.Width);
				int b = std::min(yy + blurSize, bound.Height);

				int blurPixelCount = 0;
				for (int y = yy; y < b; ++y)
				{
					const int tline = y * (this->GetPitch() / sizeof(WORD));
					auto tp = &ptr[tline];
					for (int x = xx; x < r; ++x)
					{
						Color16Struct color { *tp };

						avgR += color.R;
						avgG += color.G;
						avgB += color.B;

						++blurPixelCount;
						++tp;
					}
				}

				avgR /= blurPixelCount;
				avgG /= blurPixelCount;
				avgB /= blurPixelCount;

				WORD color = static_cast<WORD>(ColorStruct { (BYTE)avgR, (BYTE)avgG, (BYTE)avgB });
				for (int y = yy; y < b; ++y)
				{
					const int tline = y * (this->GetPitch() / sizeof(WORD));
					auto tp = &ptr[tline];
					for (int x = xx; x < r; ++x)
					{
						*tp = color;
						++tp;
					}
				}

				++p;
			}
		}

		this->Unlock();
	}
}
