#pragma once
#include <StringTable.h>
#include <CCINIClass.h>
#include <CellSpread.h>

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
	static const double GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor ArmorType);
	static int ChooseOneWeighted(const double dice, const std::vector<int>* weights);
	static bool HasHealthRatioThresholdChanged(double oldRatio, double newRatio);
	static bool ApplyTheaterSuffixToString(char* str);
	static std::string IntToDigits(int num);
	static int CountDigitsInNumber(int number);
	static CoordStruct CalculateCoordsFromDistance(CoordStruct currentCoords, CoordStruct targetCoords, int distance);
	static void DisplayDamageNumberString(int damage, DamageDisplayType type, CoordStruct coords, int& offset);
	static int GetColorFromColorAdd(int colorIndex);
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
};
