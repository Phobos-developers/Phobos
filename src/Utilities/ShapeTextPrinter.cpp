#include "ShapeTextPrinter.h"

void ShapeTextPrinter::PrintShape
(
	const char* text,
	ShapeTextPrintData& data,
	Point2D position,
	RectangleStruct& bounds,
	DSurface* pSurface,
	Point2D offset,
	BlitterFlags blitterFlags,
	int brightness,
	int tintColor
)
{
	const int length = strlen(text);
	std::vector<int> frames;

	for (int i = 0; i < length; i++)
	{
		int frame = 0;

		if (isdigit(text[i]))
		{
			frame = data.BaseNumberFrame + text[i] - '0';
		}
		else
		{
			size_t signIndex = SignSequence.find(text[i]);

			if (signIndex < SignSequence.size())
				frame = data.BaseExtraFrame + signIndex;
			else
				return;
		}

		frames.emplace_back(frame);
	}

	for (int frame : frames)
	{
		pSurface->DrawSHP
		(
			const_cast<ConvertClass*>(data.Palette),
			const_cast<SHPStruct*>(data.Shape),
			frame,
			&position,
			&bounds,
			BlitterFlags::None,
			0,
			0,
			ZGradient::Ground,
			brightness,
			tintColor,
			nullptr,
			0,
			0,
			0
		);

		position.X += data.Spacing.X;
		position.Y -= data.Spacing.Y;
	}
}
