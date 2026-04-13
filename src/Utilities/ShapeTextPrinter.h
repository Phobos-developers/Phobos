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
	int BaseExtraFrame;		// as sequence ShapeTextPrinter::SignSequence
	Vector2D<int> Spacing;

	ShapeTextPrintData(const SHPStruct* const shape, const ConvertClass* const palette, int baseNumberFrame, int baseExtraFrame, const Vector2D<int>& spacing) :
		Shape(shape),
		Palette(palette),
		BaseNumberFrame(baseNumberFrame),
		BaseExtraFrame(baseExtraFrame),
		Spacing(spacing)
	{ }
};

class ShapeTextPrinter
{
private:

	static constexpr std::string_view SignSequence = "/%$,.!?|";

public:

	static void PrintShape
	(
		const char* text,
		ShapeTextPrintData& data,
		Point2D position,
		RectangleStruct& bounds,
		DSurface* pSurface,
		Point2D offset = Point2D::Empty,
		BlitterFlags blitterFlags = BlitterFlags::None,
		int brightness = 1000,
		int tintColor = 0
	);
};
