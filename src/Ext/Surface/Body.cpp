#include "Body.h"

#include <Drawing.h>
#include <Helpers/Macro.h>

#define ENABLE_OMP 0

// https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf
void SurfaceExt::BlurRect(const RectangleStruct& rect, float blurSize)
{
	if (CLOSE_ENOUGH(blurSize, 0))
		return;
	if (this->GetBytesPerPixel() < 2)
		return;
	if (rect.Width <= 0 || rect.Height <= 0)
		return;

	const auto bound = Drawing::Intersect(rect, this->GetRect());
	if (bound.Width <= 0 || bound.Height <= 0)
		return;

	const auto line_length = this->GetPitch() / sizeof(WORD);

	auto ptr = (WORD*)this->Lock(bound.X, bound.Y);
	if (!ptr)
		return;

	// init boxes
	int box[3];
	{
		int wl = (int)std::floor(std::sqrt(4 * blurSize * blurSize + 1));
		if (wl % 2 == 0)
			--wl;
		int wu = wl + 2;

		const int m = (int)std::round((12 * blurSize * blurSize - 3 * wl * wl - 12 * wl - 9) * 1.f / (-4 * wl - 4));
		for (int i = 0; i < 3; ++i)
			box[i] = ((i < m ? wl : wu) - 1) / 2;
	}

	auto box_blur = [&](BYTE*& in, BYTE*& out, int r)
	{
		constexpr int c = 3;
		const int w = bound.Width;
		const int h = bound.Height;
		{
			float iarr = 1.f / (r + r + 1);
#if ENABLE_OMP
#pragma omp parallel for
#endif
			for (int i = 0; i < h; i++)
			{
				int ti = i * w;
				int li = ti;
				int ri = ti + r;

				int fv[3] = { in[ti * c + 0], in[ti * c + 1], in[ti * c + 2] };
				int lv[3] = { in[(ti + w - 1) * c + 0], in[(ti + w - 1) * c + 1], in[(ti + w - 1) * c + 2] };
				int val[3] = { (r + 1) * fv[0], (r + 1) * fv[1], (r + 1) * fv[2] };

				for (int j = 0; j < r; j++)
				{
					val[0] += in[(ti + j) * c + 0];
					val[1] += in[(ti + j) * c + 1];
					val[2] += in[(ti + j) * c + 2];
				}

				for (int j = 0; j <= r; j++, ri++, ti++)
				{
					val[0] += in[ri * c + 0] - fv[0];
					val[1] += in[ri * c + 1] - fv[1];
					val[2] += in[ri * c + 2] - fv[2];
					out[ti * c + 0] = static_cast<BYTE>(val[0] * iarr);
					out[ti * c + 1] = static_cast<BYTE>(val[1] * iarr);
					out[ti * c + 2] = static_cast<BYTE>(val[2] * iarr);
				}

				for (int j = r + 1; j < w - r; j++, ri++, ti++, li++)
				{
					val[0] += in[ri * c + 0] - in[li * c + 0];
					val[1] += in[ri * c + 1] - in[li * c + 1];
					val[2] += in[ri * c + 2] - in[li * c + 2];
					out[ti * c + 0] = static_cast<BYTE>(val[0] * iarr);
					out[ti * c + 1] = static_cast<BYTE>(val[1] * iarr);
					out[ti * c + 2] = static_cast<BYTE>(val[2] * iarr);
				}

				for (int j = w - r; j < w; j++, ti++, li++)
				{
					val[0] += lv[0] - in[li * c + 0];
					val[1] += lv[1] - in[li * c + 1];
					val[2] += lv[2] - in[li * c + 2];
					out[ti * c + 0] = static_cast<BYTE>(val[0] * iarr);
					out[ti * c + 1] = static_cast<BYTE>(val[1] * iarr);
					out[ti * c + 2] = static_cast<BYTE>(val[2] * iarr);
				}
			}
		} // horizonal blur
		std::swap(in, out);
		{
			float iarr = 1.f / (r + r + 1);
#if ENABLE_OMP
#pragma omp parallel for
#endif
			for (int i = 0; i < w; i++)
			{
				int ti = i;
				int li = ti;
				int ri = ti + r * w;

				int fv[3] = { in[ti * c + 0], in[ti * c + 1], in[ti * c + 2] };
				int lv[3] = { in[(ti + w * (h - 1)) * c + 0], in[(ti + w * (h - 1)) * c + 1], in[(ti + w * (h - 1)) * c + 2] };
				int val[3] = { (r + 1) * fv[0], (r + 1) * fv[1], (r + 1) * fv[2] };

				for (int j = 0; j < r; j++)
				{
					val[0] += in[(ti + j * w) * c + 0];
					val[1] += in[(ti + j * w) * c + 1];
					val[2] += in[(ti + j * w) * c + 2];
				}

				for (int j = 0; j <= r; j++, ri += w, ti += w)
				{
					val[0] += in[ri * c + 0] - fv[0];
					val[1] += in[ri * c + 1] - fv[1];
					val[2] += in[ri * c + 2] - fv[2];
					out[ti * c + 0] = static_cast<BYTE>(val[0] * iarr);
					out[ti * c + 1] = static_cast<BYTE>(val[1] * iarr);
					out[ti * c + 2] = static_cast<BYTE>(val[2] * iarr);
				}

				for (int j = r + 1; j < h - r; j++, ri += w, ti += w, li += w)
				{
					val[0] += in[ri * c + 0] - in[li * c + 0];
					val[1] += in[ri * c + 1] - in[li * c + 1];
					val[2] += in[ri * c + 2] - in[li * c + 2];
					out[ti * c + 0] = static_cast<BYTE>(val[0] * iarr);
					out[ti * c + 1] = static_cast<BYTE>(val[1] * iarr);
					out[ti * c + 2] = static_cast<BYTE>(val[2] * iarr);
				}

				for (int j = h - r; j < h; j++, ti += w, li += w)
				{
					val[0] += lv[0] - in[li * c + 0];
					val[1] += lv[1] - in[li * c + 1];
					val[2] += lv[2] - in[li * c + 2];
					out[ti * c + 0] = static_cast<BYTE>(val[0] * iarr);
					out[ti * c + 1] = static_cast<BYTE>(val[1] * iarr);
					out[ti * c + 2] = static_cast<BYTE>(val[2] * iarr);
				}
			}
		} // total blur
	};

	size_t size = bound.Width * bound.Height * 3;
	BYTE* in = new BYTE[size];
	BYTE* out = new BYTE[size];

	// write surface data to in
	{
		auto d = in;
		auto p = ptr;
		for (int y = 0; y < bound.Height; ++y)
		{
			auto q = p;
			for (int x = 0; x < bound.Width; ++x)
			{
				Drawing::Int_To_RGB(*q, d[0], d[1], d[2]);
				d += 3;
				++q;
			}
			p += line_length;
		}
	}

	box_blur(in, out, box[0]);
	box_blur(out, in, box[1]);
	box_blur(in, out, box[2]);

	// write out to surface data
	{
		auto d = out;
		auto p = ptr;
		for (int y = 0; y < bound.Height; ++y)
		{
			auto q = p;
			for (int x = 0; x < bound.Width; ++x)
			{
				*q = (WORD)Drawing::RGB_To_Int(d[0], d[1], d[2]);
				d += 3;
				++q;
			}
			p += line_length;
		}
	}

	delete[] in;
	delete[] out;

	this->Unlock();
}
