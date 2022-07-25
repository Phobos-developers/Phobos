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
	Point2D posDraw,
	RectangleStruct& rBound,
	DSurface* pSurface,
	Point2D offset,
	BlitterFlags eBlitterFlags,
	int iBright,
	int iTintColor
)
{
	int iLength = strlen(text);
	std::vector<int> vFrames;

	for (int i = 0; i < iLength; i++)
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
				frame = data.BaseSignFrame + signIndex;
			else
				return;
		}

		vFrames.emplace_back(frame);
	}

	for (int frame : vFrames)
	{
		pSurface->DrawSHP
		(
			const_cast<ConvertClass*>(data.Palette),
			const_cast<SHPStruct*>(data.Shape),
			frame,
			&posDraw,
			&rBound,
			BlitterFlags::None,
			0,
			0,
			ZGradient::Ground,
			1000,
			0,
			nullptr,
			0,
			0,
			0
		);

		posDraw += data.Interval;
	}
}
