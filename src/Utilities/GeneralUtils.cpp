#include "GeneralUtils.h"
#include "Debug.h"
#include <Theater.h>
#include <ScenarioClass.h>
#include <BitFont.h>

#include <Ext/Rules/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/Constructs.h>

bool GeneralUtils::IsValidString(const char* str)
{
	return str != nullptr
		&& strlen(str) != 0
		&& !INIClass::IsBlank(str);
}

void GeneralUtils::IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min, int max)
{
	if (*source < min || *source>max)
	{
		Debug::Log("[Developer warning][%s]%s=%d is invalid! Reset to %d.\n", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

void GeneralUtils::DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min, double max)
{
	if (*source < min || *source>max)
	{
		Debug::Log("[Developer warning][%s]%s=%f is invalid! Reset to %f.\n", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

const wchar_t* GeneralUtils::LoadStringOrDefault(const char* key, const wchar_t* defaultValue)
{
	if (GeneralUtils::IsValidString(key))
		return StringTable::LoadString(key);
	else
		return defaultValue;
}

const wchar_t* GeneralUtils::LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue)
{
	return wcsstr(LoadStringOrDefault(key, defaultValue), L"MISSING:") ? defaultValue : LoadStringOrDefault(key, defaultValue);
}

std::vector<CellStruct> GeneralUtils::AdjacentCellsInRange(unsigned int range)
{
	std::vector<CellStruct> result;

	for (CellSpreadEnumerator it(range); it; ++it)
		result.push_back(*it);

	return result;
}

const int GeneralUtils::GetRangedRandomOrSingleValue(PartialVector2D<int> range)
{
	return range.X >= range.Y || range.ValueCount < 2 ? range.X : ScenarioClass::Instance->Random.RandomRanged(range.X, range.Y);
}

const double GeneralUtils::GetRangedRandomOrSingleValue(PartialVector2D<double> range)
{
	int min = static_cast<int>(range.X * 100);
	int max = static_cast<int>(range.Y * 100);

	return range.X >= range.Y || range.ValueCount < 2 ? range.X : (ScenarioClass::Instance->Random.RandomRanged(min, max) / 100.0);
}

const double GeneralUtils::GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor ArmorType)
{
	return double(MapClass::GetTotalDamage(100, pWH, ArmorType, 0)) / 100.0;
}

// Weighted random element choice (weight) - roll for one.
// Takes a vector of integer type weights, which are then summed to calculate the chances.
// Returns chosen index or -1 if nothing is chosen.
int GeneralUtils::ChooseOneWeighted(const double dice, const std::vector<int>* weights)
{
	float sum = 0.0;
	float sum2 = 0.0;

	for (size_t i = 0; i < weights->size(); i++)
		sum += (*weights)[i];

	for (size_t i = 0; i < weights->size(); i++)
	{
		sum2 += (*weights)[i];
		if (dice < (sum2 / sum))
			return i;
	}

	return -1;
}

// Checks if health ratio has changed threshold (Healthy/ConditionYellow/Red).
bool GeneralUtils::HasHealthRatioThresholdChanged(double oldRatio, double newRatio)
{
	if (oldRatio == newRatio)
		return false;

	if (oldRatio > RulesClass::Instance->ConditionYellow && newRatio <= RulesClass::Instance->ConditionYellow)
	{
		return true;
	}
	else if (oldRatio <= RulesClass::Instance->ConditionYellow && oldRatio > RulesClass::Instance->ConditionRed &&
		(newRatio <= RulesClass::Instance->ConditionRed || newRatio > RulesClass::Instance->ConditionYellow))
	{
		return true;
	}
	else if (oldRatio <= RulesClass::Instance->ConditionRed && newRatio > RulesClass::Instance->ConditionRed)
	{
		return true;
	}

	return false;
}

bool GeneralUtils::ApplyTheaterSuffixToString(char* str)
{
	if (auto pSuffix = strstr(str, "~~~"))
	{
		auto theater = ScenarioClass::Instance->Theater;
		auto pExtension = Theater::GetTheater(theater).Extension;
		pSuffix[0] = pExtension[0];
		pSuffix[1] = pExtension[1];
		pSuffix[2] = pExtension[2];
		return true;
	}

	return false;
}

std::string GeneralUtils::IntToDigits(int num)
{
	std::string digits;

	if (num == 0)
	{
		digits.push_back('0');
		return digits;
	}

	while (num)
	{
		digits.push_back(static_cast<char>(num % 10) + '0');
		num /= 10;
	}

	std::reverse(digits.begin(), digits.end());

	return digits;
}

int GeneralUtils::CountDigitsInNumber(int number)
{
	int digits = 0;

	while (number)
	{
		number /= 10;
		digits++;
	}

	return digits;
}

// Calculates a new coordinates based on current & target coordinates within specified distance (can be negative to switch the direction) in leptons.
CoordStruct GeneralUtils::CalculateCoordsFromDistance(CoordStruct currentCoords, CoordStruct targetCoords, int distance)
{
	int deltaX = currentCoords.X - targetCoords.X;
	int deltaY = targetCoords.Y - currentCoords.Y;

	double atan = Math::atan2(deltaY, deltaX);
	double radians = (((atan - Math::HalfPi) * (1.0 / Math::GameDegreesToRadiansCoefficient)) - Math::GameDegrees90) * Math::GameDegreesToRadiansCoefficient;
	int x = static_cast<int>(targetCoords.X + Math::cos(radians) * distance);
	int y = static_cast<int>(targetCoords.Y - Math::sin(radians) * distance);

	return CoordStruct { x, y, targetCoords.Z };
}

void GeneralUtils::DisplayDamageNumberString(int damage, DamageDisplayType type, CoordStruct coords, int& offset)
{
	if (damage == 0)
		return;

	ColorStruct color;

	switch (type)
	{
	case DamageDisplayType::Regular:
		color = damage > 0 ? ColorStruct { 255, 0, 0 } : ColorStruct { 0, 255, 0 };
		break;
	case DamageDisplayType::Shield:
		color = damage > 0 ? ColorStruct { 0, 160, 255 } : ColorStruct { 0, 255, 230 };
		break;
	case DamageDisplayType::Intercept:
		color = damage > 0 ? ColorStruct { 255, 128, 128 } : ColorStruct { 128, 255, 128 };
		break;
	default:
		break;
	}

	int maxOffset = Unsorted::CellWidthInPixels / 2;
	int width = 0, height = 0;
	wchar_t damageStr[0x20];
	swprintf_s(damageStr, L"%d", damage);

	BitFont::Instance->GetTextDimension(damageStr, &width, &height, 120);

	if (offset >= maxOffset || offset == INT32_MIN)
		offset = -maxOffset;

	FlyingStrings::Add(damageStr, coords, color, Point2D { offset - (width / 2), 0 });

	offset = offset + width;
}

DynamicVectorClass<ColorScheme*>* GeneralUtils::BuildPalette(const char* paletteFileName)
{
	if (GeneralUtils::IsValidString(paletteFileName))
	{
		char pFilename[0x20];
		strcpy_s(pFilename, paletteFileName);

		return ColorScheme::GeneratePalette(pFilename);
	}

	return nullptr;
}

// Gets integer representation of color from ColorAdd corresponding to given index, or 0 if there's no color found.
// Code is pulled straight from game's draw functions that deal with the tint colors.
int GeneralUtils::GetColorFromColorAdd(int colorIndex)
{
	auto const& colorAdd = RulesClass::Instance->ColorAdd;
	int colorValue = 0;

	if (colorIndex < 0 || colorIndex >= (sizeof(colorAdd) / sizeof(ColorStruct)))
		return colorValue;

	auto const& color = colorAdd[colorIndex];

	if (RulesExt::Global()->ColorAddUse8BitRGB)
		return Drawing::RGB_To_Int(color);

	int red = color.R;
	int green = color.G;
	int blue = color.B;

	if (Drawing::ColorMode() == RGBMode::RGB565)
		colorValue |= blue | (32 * (green | (red << 6)));

	if (Drawing::ColorMode() != RGBMode::RGB655)
		colorValue |= blue | (((32 * red) | (green >> 1)) << 6);

	colorValue |= blue | (32 * ((32 * red) | (green >> 1)));

	return colorValue;
}
