#include "Anchor.h"

#include <GeneralDefinitions.h>


double Anchor::GetRelativeOffsetHorizontal() const
{
	// Enum goes from 0 to 2 from left to right. Cast it and divide it
	// by 2 and you get the percentage. Pretty clever huh? - Kerbiter
	return (static_cast<double>(this->Horizontal.Get()) / 2.0);
}

double Anchor::GetRelativeOffsetVertical() const
{
	// Same deal as with the left-right one - Kerbiter
	return (static_cast<double>(this->Vertical.Get()) / 2.0);
}

Point2D Anchor::OffsetPosition(
	Point2D& const topLeft, Point2D& const topRight,
	Point2D& const bottomLeft) const
{
	Point2D result { topLeft };
	Point2D deltaTopRight { topRight - topLeft };
	Point2D deltaBottomLeft { bottomLeft - topLeft };

	result += deltaTopRight * this->GetRelativeOffsetHorizontal();
	result += deltaBottomLeft * this->GetRelativeOffsetVertical();

	return result;
}


Point2D Anchor::OffsetPosition(RectangleStruct& const rect) const
{
	Point2D result { rect.X, rect.Y };

	result.X += rect.Width * this->GetRelativeOffsetHorizontal();
	result.Y += rect.Height * this->GetRelativeOffsetVertical();

	return result;
}

Point2D Anchor::OffsetPosition(LTRBStruct& const ltrb) const
{
	Point2D result { ltrb.Left, ltrb.Top };
	int deltaX = ltrb.Right - ltrb.Left;
	int deltaY = ltrb.Bottom - ltrb.Top;

	result.X += deltaX * this->GetRelativeOffsetHorizontal();
	result.Y += deltaY * this->GetRelativeOffsetVertical();

	return result;
}


void Anchor::Read(INI_EX& parser, const char* pSection, const char* pFlagFormat)
{
	char flagName[0x40];

	_snprintf_s(flagName, _TRUNCATE, pFlagFormat, "Horizontal");
	this->Horizontal.Read(parser, pSection, flagName);

	_snprintf_s(flagName, _TRUNCATE, pFlagFormat, "Vertical");
	this->Vertical.Read(parser, pSection, flagName);
}

bool Anchor::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool Anchor::Save(PhobosStreamWriter& stm) const
{
	return const_cast<Anchor*>(this)->Serialize(stm);
}

template <typename T>
bool Anchor::Serialize(T& stm)
{
	return stm
		.Process(this->Horizontal)
		.Process(this->Vertical)
		.Success();
}