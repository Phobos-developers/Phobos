#pragma once
#include <StringTable.h>
#include <CCINIClass.h>
#include <CellSpread.h>

#include <Helpers/Iterators.h>
#include <Helpers/Enumerators.h>

#include <string.h>
#include <iterator>
#include <vector>

#define MIN(x) std::numeric_limits<x>::min()
#define MAX(x) std::numeric_limits<x>::max()

class GeneralUtils
{
public:
	static bool IsValidString(const char* str);
	static void IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min = MIN(int), int max = MAX(int));
	static void DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min = MIN(double), double max = MAX(double));
	static const wchar_t* LoadStringOrDefault(char* key, const wchar_t* defaultValue);
	static const wchar_t* LoadStringUnlessMissing(char* key, const wchar_t* defaultValue);
	static std::vector<CellStruct> AdjacentCellsInRange(unsigned int range);
	static const int GetRangedRandomOrSingleValue(Point2D range);
	static const double GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor ArmorType);
	static int ChooseOneWeighted(const double dice, const std::vector<int>* weights);
	static double FastPow(double x, double n);
	static bool HasHealthRatioThresholdChanged(double oldRatio, double newRatio);
};
