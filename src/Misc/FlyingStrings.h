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
		int XOffset;
		int CreationFrame;
		wchar_t Text[0x20];
		COLORREF Color;

	};

	static const int Duration = 75;
	static const int MaxCycleWidth = 30;
	static int CurrentCycleWidth;
	static std::vector<Item> Data;

	static bool DrawAllowed(CoordStruct &nCoords);

public:
	static void Add(const wchar_t *text, CoordStruct coords, ColorStruct color, bool isCyclic = false);
	static void UpdateAll();
};
