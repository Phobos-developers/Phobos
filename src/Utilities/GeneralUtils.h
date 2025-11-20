#pragma once
#include <StringTable.h>
#include <CCINIClass.h>
#include <CellSpread.h>
#include <Conversions.h>

#include <Helpers/Iterators.h>
#include <Helpers/Enumerators.h>
#include <Utilities/Enum.h>

#include <string.h>
#include <iterator>
#include <vector>
#include <string>

#include "Template.h"

#define MIN(x) std::numeric_limits<x>::min()
#define MAX(x) std::numeric_limits<x>::max()

class GeneralUtils
{
public:
	static bool IsValidString(const char* str);
	static void IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min = MIN(int), int max = MAX(int));
	static void DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min = MIN(double), double max = MAX(double));
	static const wchar_t* LoadStringOrDefault(const char* key, const wchar_t* defaultValue);
	static const wchar_t* LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue);
	static std::vector<CellStruct> AdjacentCellsInRange(unsigned int range);
	static const int GetRangedRandomOrSingleValue(PartialVector2D<int> range);
	static const double GetRangedRandomOrSingleValue(PartialVector2D<double> range);
	static const double GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor armorType);
	static const double GetWarheadVersusArmor(WarheadTypeClass* pWH, TechnoClass* pThis, TechnoTypeClass* pType = nullptr);
	static int ChooseOneWeighted(const double dice, const std::vector<int>* weights);
	static bool HasHealthRatioThresholdChanged(double oldRatio, double newRatio);
	static bool ApplyTheaterSuffixToString(char* str);
	static std::string IntToDigits(int num);
	static int CountDigitsInNumber(int number);
	static CoordStruct CalculateCoordsFromDistance(CoordStruct currentCoords, CoordStruct targetCoords, int distance);
	static void DisplayDamageNumberString(int damage, DamageDisplayType type, CoordStruct coords, int& offset);
	static int GetColorFromColorAdd(int colorIndex);
	static int SafeMultiply(int value, int mult);
	static int SafeMultiply(int value, double mult);
	static DynamicVectorClass<ColorScheme*>* BuildPalette(const char* paletteFileName);

	template<typename T>
	static constexpr T FastPow(T x, size_t n)
	{
		// Real fast pow calc x^n in O(log(n))
		T result = 1;
		T base = x;
		while (n)
		{
			if (n & 1) result *= base;
			base *= base;
			n >>= 1;
		}
		return result;
	}

	// Returns item from vector based on given direction and number of items in the vector, f.ex directional animations.
	// Vector is expected to have 2^n items where n >= 3 and n <= 16 for the logic to work correctly, other cases return first item.
	// Do not pass an empty vector, size/indices are not sanity checked here.
	template<typename T>
	static T GetItemForDirection(std::vector<T> const& items, DirStruct const& direction)
	{
		// Log base 2
		unsigned int bitsTo = Conversions::Int2Highest(static_cast<int>(items.size()));

		if (bitsTo >= 3 && bitsTo <= 16)
		{
			// Same shit as DirStruct::TranslateFixedPoint().
			// Because it uses template args and it is necessary to use
			// non-compile time values here, it is duplicated & inlined.
			unsigned int index = direction.Raw;
			const unsigned int offset = 1 << (bitsTo - 3);
			const unsigned int bitsFrom = 16;
			const unsigned int maskIn = ((1 << bitsFrom) - 1);
			const unsigned int maskOut = (1 << bitsTo) - 1;

			if (bitsFrom > bitsTo)
				index = (((((index & maskIn) >> (bitsFrom - bitsTo - 1)) + 1) >> 1) + offset) & maskOut;
			else if (bitsFrom < bitsTo)
				index = (((index - offset) & maskIn) << (bitsTo - bitsFrom)) & maskOut;
			else
				index = index & maskOut;

			return items[index];
		}

		return items[0];
	}
};
