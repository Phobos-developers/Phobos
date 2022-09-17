/*FlyingStrings.h
Useable to get out messages from units
Used to output Bounty messages
By AlexB and Joshy
*/

#pragma once
#include <vector>
#include <ColorScheme.h>

class FlyingStrings
{
private:

	struct Item
	{
		CoordStruct Location;
		Point2D PixelOffset;
		int CreationFrame;
		wchar_t Text[0x20];
		COLORREF Color;

	};

	static const int Duration = 75;
	static std::vector<Item> Data;

	static bool DrawAllowed(CoordStruct &nCoords);

public:
	static void Add(const wchar_t *text, CoordStruct coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty);
	static void UpdateAll();
};
