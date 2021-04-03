#pragma once
#include <StringTable.h>
#include <CCINIClass.h>
#include <CellSpread.h>

#include <Helpers/Iterators.h>
#include <Helpers/Enumerators.h>

#include <string.h>
#include <iterator>
#include <vector>

class GeneralUtils
{
public:
    static bool IsValidString(const char* str);
    static const wchar_t* LoadStringOrDefault(char* key, const wchar_t* defaultValue);
    static const wchar_t* LoadStringUnlessMissing(char* key, const wchar_t* defaultValue);
    static std::vector<CellStruct> AdjacentCellsInRange(unsigned int range);
};