#pragma once
#include <Utilities/Enum.h>
#include <Utilities/TemplateDef.h>

#include <YRPPCore.h>

// Helper class to get anchor points on an arbitrary rectangle or a parallelogram
class Anchor
{
public:
	Valueable<HorizontalPosition> Horizontal { HorizontalPosition::Left };
	Valueable<VerticalPosition>   Vertical { VerticalPosition::Top };

	Anchor(HorizontalPosition hPos, VerticalPosition vPos)
		: Horizontal { hPos }, Vertical { vPos }
	{ }

	// Maps enum values to offset relative to width
	double GetRelativeOffsetHorizontal() const;
	// Maps enum values to offset relative to height
	double GetRelativeOffsetVertical() const;

	// Get an anchor point for a freeform parallelogram
	Point2D OffsetPosition(
		const Point2D& topLeft,
		const Point2D& topRight,
		const Point2D& bottomLeft
	) const;

	Point2D OffsetPosition(const RectangleStruct& rect) const;
	Point2D OffsetPosition(const LTRBStruct& ltrb) const;

	void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};
