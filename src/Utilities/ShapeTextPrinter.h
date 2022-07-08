#pragma once
#include <Utilities/TemplateDef.h>

// only numbers and sign
class ShapeTextPrintData
{
public:
	// Shape
	const SHPStruct* const Shape;
	const ConvertClass* const Palette;
	int BaseNumberFrame;	// frame index of 0
	int BaseSignFrame;		// as sequence ShapeTextPrinter::SignSequence
	Vector2D<int> Interval;

	ShapeTextPrintData(const SHPStruct* const shape, const ConvertClass* const palette, int iBaseNumberFrame, int iBaseSignFrame, const Vector2D<int>& vInterval)
		: Shape(shape), Palette(palette), BaseNumberFrame(iBaseNumberFrame), BaseSignFrame(iBaseSignFrame), Interval(vInterval)
	{ }
};

class ShapeTextPrinter
{
private:

	static const char* SignSequence;
	static const int SignSequenceLength;

	static int GetSignIndex(const char sign);

public:

	static void PrintShape
	(
		const char* text,
		ShapeTextPrintData& data,
		Point2D posDraw,
		RectangleStruct& rBound,
		DSurface* pSurface,
		Point2D offset = Point2D::Empty,
		BlitterFlags eBlitterFlags = BlitterFlags::None,
		int iBright = 1000,
		int iTintColor = 0
	);
};
