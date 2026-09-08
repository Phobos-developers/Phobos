#include "Constructs.h"
#include "GeneralUtils.h"
#include "Debug.h"
#include <Theater.h>
#include <BitFont.h>

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Misc/FlyingStrings.h>
#include "AresHelper.h"

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
	result.reserve((2 * range + 1) * (2 * range + 1));

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
	const int min = static_cast<int>(range.X * 100);
	const int max = static_cast<int>(range.Y * 100);

	return range.X >= range.Y || range.ValueCount < 2 ? range.X : (ScenarioClass::Instance->Random.RandomRanged(min, max) / 100.0);
}

struct VersesData
{
	double Verses;
	WarheadFlags Flags;
};

struct DummyTypeExtHere
{
	char _[0x24];
	std::vector<VersesData> Verses;
};

const double GeneralUtils::GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor armorType)
{
	if (AresHelper::CanUseAres)
		return reinterpret_cast<DummyTypeExtHere*>(*(uintptr_t*)((char*)pWH + 0x1CC))->Verses[static_cast<int>(armorType)].Verses;

	return static_cast<double>(MapClass::GetTotalDamage(100, pWH, armorType, 0)) / 100.0;
}

const double GeneralUtils::GetWarheadVersusArmor(WarheadTypeClass* pWH, TechnoClass* pThis, TechnoTypeClass* pType)
{
	auto armorType = pType->Armor;
	auto const pShield = TechnoExt::Fetch(pThis)->Shield.get();

	if (pShield && pShield->IsActive() && !pShield->CanBePenetrated(pWH))
		armorType = pShield->GetArmorType(pType);

	return GeneralUtils::GetWarheadVersusArmor(pWH, armorType);
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

	if (oldRatio > RulesClass::Instance->ConditionYellow
		&& newRatio <= RulesClass::Instance->ConditionYellow)
	{
		return true;
	}
	else if (oldRatio <= RulesClass::Instance->ConditionYellow
		&& oldRatio > RulesClass::Instance->ConditionRed
		&& (newRatio <= RulesClass::Instance->ConditionRed || newRatio > RulesClass::Instance->ConditionYellow))
	{
		return true;
	}
	else if (oldRatio <= RulesClass::Instance->ConditionRed
		&& newRatio > RulesClass::Instance->ConditionRed)
	{
		return true;
	}

	return false;
}

bool GeneralUtils::ApplyTheaterSuffixToString(char* str)
{
	if (auto pSuffix = strstr(str, "~~~"))
	{
		const auto theater = ScenarioClass::Instance->Theater;
		const auto pExtension = Theater::GetTheater(theater).Extension;
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
	digits.reserve(10); // 32-bit int max: 2,147,483,647 (10 digits)

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

// Calculates direction between two coordinates.
DirStruct GeneralUtils::GetDirectionBetweenCoords(const CoordStruct& currentCoords, const CoordStruct& targetCoords)
{
	const int deltaX = targetCoords.X - currentCoords.X;
	const int deltaY = currentCoords.Y - targetCoords.Y;
	const double atan = Math::atan2(deltaY, deltaX);
	const double radians = (((atan - Math::HalfPi) * (1.0 / Math::GameDegreesToRadiansCoefficient)) - Math::GameDegrees90) * Math::GameDegreesToRadiansCoefficient;
	DirStruct dir {};
	dir.SetRadian<65536>(radians);
	return dir;
}

// Calculates a new coordinates based on current & target coordinates within specified distance (can be negative to switch the direction) in leptons.
CoordStruct GeneralUtils::CalculateCoordsFromDistance(const CoordStruct& currentCoords, const CoordStruct& targetCoords, int distance)
{
	const double radians = GeneralUtils::GetDirectionBetweenCoords(currentCoords, targetCoords).GetRadian<65536>() + Math::Pi;
	const int x = static_cast<int>(targetCoords.X + Math::cos(radians) * distance);
	const int y = static_cast<int>(targetCoords.Y - Math::sin(radians) * distance);

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

	const int maxOffset = Unsorted::CellWidthInPixels / 2;
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

	const int red = color.R;
	const int green = color.G;
	const int blue = color.B;

	switch (Drawing::ColorMode)
	{
	case RGBMode::RGB565:
		colorValue |= (red << 6 | green) << 5 | blue;
		break;
	case RGBMode::RGB556:
		colorValue |= (red << 5 | green >> 1) << 6 | blue;
		break;
	default:
		colorValue |= (red << 5 | green >> 1) << 5 | blue;
		break;
	}

	return colorValue;
}

int GeneralUtils::SafeMultiply(int value, int mult)
{
	long long product = static_cast<long long>(value) * mult;

	if (product > INT32_MAX)
		product = INT32_MAX;
	else if (product < INT32_MIN)
		product = INT32_MIN;

	return static_cast<int>(product);
}

int GeneralUtils::SafeMultiply(int value, double mult)
{
	double product = static_cast<double>(value) * mult;

	if (product > INT32_MAX)
		product = INT32_MAX;
	else if (product < INT32_MIN)
		product = INT32_MIN;

	return static_cast<int>(product);
}

// SHP & PCX drawing support in the same function
bool GeneralUtils::DrawImage(
	DSurface* pSurface,
	RectangleStruct destinationRect,
	BSurface* pPCXSurface,
	SHPStruct* fileSHP,
	ConvertClass* pPalette,
	int frameIndex,
	int zAdjust,
	BlitterFlags blitterFlags)
{
	if (!pSurface || (!pPCXSurface && !fileSHP))
		return false;

	bool painted = false;

	// Prioritize drawing the PCX file if it's provided
	if (pPCXSurface)
	{
		// This function handles stretching the PCX to fit the destinationRect
		PCX::Instance.BlitToSurface(&destinationRect, pSurface, pPCXSurface);
		painted = true;
	}
	// Otherwise, if an SHP is provided, draw it
	else if (fileSHP)
	{
		// SHP drawing requires a palette converter
		if (!pPalette)
		{
			Debug::Log("DrawImage Error: Attempted to draw SHP without providing a pPalette.\n");
			return false;
		}

		Point2D noLocation = { 0, 0 };

		CC_Draw_Shape(
			pSurface,
			pPalette,
			fileSHP,
			frameIndex,
			&noLocation,
			&destinationRect,
			BlitterFlags::None,
			0, zAdjust, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0
		);
		painted = true;
	}

	// Use the Phobos PCX instance to blit the image
	if (painted && blitterFlags == (BlitterFlags::Darken | BlitterFlags::bf_400))
	{
		auto black = ColorStruct { 0, 0, 0 };
		int opacity = 40;
		pSurface->FillRectTrans(&destinationRect, &black, opacity);
	}

	// Other new BlitterFlags cases should be placed here so both SHP & PCS will be affected
	return true;
}

std::unique_ptr<std::vector<PhobosPCXFile>> GeneralUtils::GetAnimationPCX(const std::string& baseFilename)
{
	auto animationFrames = std::make_unique<std::vector<PhobosPCXFile>>();

	std::string filenameBase = baseFilename;
	std::string extension = ".PCX";

	// Find the position of the last dot to separate the extension
	size_t lastDot = baseFilename.find_last_of('.');
	if (lastDot != std::string::npos)
	{
		filenameBase = baseFilename.substr(0, lastDot);
		extension = baseFilename.substr(lastDot);
	}

	// Check if the part before the extension was a frame number and remove it if so
	if (filenameBase.length() > 5 && filenameBase[filenameBase.length() - 5] == ' ')
	{
		std::string frameNumberStr = filenameBase.substr(filenameBase.length() - 4);
		bool isNumeric = true;
		for (char c : frameNumberStr)
		{
			if (!isdigit(c))
			{
				isNumeric = false;
				break;
			}
		}
		if (isNumeric)
		{
			filenameBase = filenameBase.substr(0, filenameBase.length() - 5);
		}
	}

	// Try loading frame 0 as "<base> 0000.<ext>" first
	char frame0Filename[256];
	_snprintf_s(frame0Filename, sizeof(frame0Filename), "%s 0000%s", filenameBase.c_str(), extension.c_str());
	PhobosPCXFile frame0(frame0Filename);
	if (frame0.Exists())
	{
		animationFrames->emplace_back(std::move(frame0));
	}
	else
	{
		// If "<base> 0000.<ext>" doesn't exist, try loading the exact filename as provided (e.g. "LOADOUT.PCX")
		PhobosPCXFile exactFile(baseFilename.c_str());
		if (exactFile.Exists())
		{
			animationFrames->emplace_back(std::move(exactFile));
		}
		else
		{
			return animationFrames;
		}
	}

	// Loop to find and load the subsequent frames, starting from frame 1
	for (int i = 1; i < 10000; ++i)
	{
		char currentFilename[256];
		_snprintf_s(currentFilename, sizeof(currentFilename), "%s %04d%s", filenameBase.c_str(), i, extension.c_str());

		PhobosPCXFile filePCX(currentFilename);
		if (filePCX.Exists())
			animationFrames->emplace_back(std::move(filePCX));
		else
			break;
	}

	return animationFrames;
}
