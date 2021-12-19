#include "Body.h"

#include <SuperClass.h>

DEFINE_HOOK(0x6851AC, LoadGame_InitStuff_ApplyLighting, 0x5)
{
	std::swap(ScenarioExt::Global()->InitialLightingData.AmbientOriginal, ScenarioClass::Instance->AmbientOriginal);
	std::swap(ScenarioExt::Global()->InitialLightingData.AmbientCurrent, ScenarioClass::Instance->AmbientCurrent);
	std::swap(ScenarioExt::Global()->InitialLightingData.AmbientTarget, ScenarioClass::Instance->AmbientTarget);
	std::swap(ScenarioExt::Global()->InitialLightingData.Red, ScenarioClass::Instance->Red);
	std::swap(ScenarioExt::Global()->InitialLightingData.Green, ScenarioClass::Instance->Green);
	std::swap(ScenarioExt::Global()->InitialLightingData.Blue, ScenarioClass::Instance->Blue);
	std::swap(ScenarioExt::Global()->InitialLightingData.Ground, ScenarioClass::Instance->Ground);
	std::swap(ScenarioExt::Global()->InitialLightingData.Level, ScenarioClass::Instance->Level);

	// We need to use map default lighting to initialize the cells
	// Otherwise the map lighitng will looks corrupted
	MapClass::Instance->CellIteratorReset();
	for (auto pCell = MapClass::Instance->CellIteratorNext(); pCell; pCell = MapClass::Instance->CellIteratorNext())
	{
		if (pCell->LightConvert)
			GameDelete(pCell->LightConvert);

		pCell->InitLightConvert();
	}

	LightningStorm::Init();

	std::swap(ScenarioExt::Global()->InitialLightingData.AmbientOriginal, ScenarioClass::Instance->AmbientOriginal);
	std::swap(ScenarioExt::Global()->InitialLightingData.AmbientCurrent, ScenarioClass::Instance->AmbientCurrent);
	std::swap(ScenarioExt::Global()->InitialLightingData.AmbientTarget, ScenarioClass::Instance->AmbientTarget);
	std::swap(ScenarioExt::Global()->InitialLightingData.Red, ScenarioClass::Instance->Red);
	std::swap(ScenarioExt::Global()->InitialLightingData.Green, ScenarioClass::Instance->Green);
	std::swap(ScenarioExt::Global()->InitialLightingData.Blue, ScenarioClass::Instance->Blue);
	std::swap(ScenarioExt::Global()->InitialLightingData.Ground, ScenarioClass::Instance->Ground);
	std::swap(ScenarioExt::Global()->InitialLightingData.Level, ScenarioClass::Instance->Level);

	// Restore colors
	for (auto& pLightConvert : *LightConvertClass::Array)
		pLightConvert->UpdateColors(
			ScenarioExt::Global()->RetintTiles.Red,
			ScenarioExt::Global()->RetintTiles.Green,
			ScenarioExt::Global()->RetintTiles.Blue,
			false);

	for (auto& pScheme : *ColorScheme::Array)
		pScheme->LightConvert->UpdateColors(
			ScenarioExt::Global()->RetintSchemes.Red,
			ScenarioExt::Global()->RetintSchemes.Green,
			ScenarioExt::Global()->RetintSchemes.Blue,
			false);

	ScenarioClass::UpdateHashPalLighting(
		ScenarioExt::Global()->RetintHashes.Red,
		ScenarioExt::Global()->RetintHashes.Green,
		ScenarioExt::Global()->RetintHashes.Blue,
		false);

	ScenarioClass::UpdateCellLighting();

	return 0x6851B1;
}