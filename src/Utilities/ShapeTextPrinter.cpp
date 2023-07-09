#include "ShapeTextPrinter.h"

const char* ShapeTextPrinter::SignSequence = "/%$,.!?|";
const int ShapeTextPrinter::SignSequenceLength = strlen(ShapeTextPrinter::SignSequence);

int ShapeTextPrinter::GetSignIndex(const char sign)
{
	return (std::find(SignSequence, SignSequence + SignSequenceLength, sign) - SignSequence);
}

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
			int signIndex = GetSignIndex(text[i]);

			if (signIndex < SignSequenceLength)
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
