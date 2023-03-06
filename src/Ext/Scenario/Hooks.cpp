#include "Body.h"

DEFINE_HOOK(0x6851AC, ScenarioClass_LoadGame_Initialize_IonStormClass, 0x5)
{
	auto swap_data = []()
	{
		std::swap(ScenarioExt::Global()->DefaultAmbientOriginal, ScenarioClass::Instance->AmbientOriginal);
		std::swap(ScenarioExt::Global()->DefaultAmbientCurrent, ScenarioClass::Instance->AmbientCurrent);
		std::swap(ScenarioExt::Global()->DefaultAmbientTarget, ScenarioClass::Instance->AmbientTarget);
		std::swap(ScenarioExt::Global()->DefaultNormalLighting, ScenarioClass::Instance->NormalLighting);
	};

	swap_data();

	MapClass::Instance->CellIteratorReset();
	for (auto pCell = MapClass::Instance->CellIteratorNext(); pCell; pCell = MapClass::Instance->CellIteratorNext())
	{
		if (pCell->LightConvert)
			GameDelete(pCell->LightConvert);

		pCell->InitLightConvert();
	}

	swap_data();


	for (auto& pLightConvert : *LightConvertClass::Array)
		pLightConvert->UpdateColors(
			ScenarioExt::Global()->CurrentTint_Tiles.Red * 10,
			ScenarioExt::Global()->CurrentTint_Tiles.Green * 10,
			ScenarioExt::Global()->CurrentTint_Tiles.Blue * 10,
			false);

	if (ScenarioExt::Global()->CurrentTint_Schemes != TintStruct { -1,-1,-1 })
	{
		for (auto& pScheme : *ColorScheme::Array)
			pScheme->LightConvert->UpdateColors(
				ScenarioExt::Global()->CurrentTint_Schemes.Red * 10,
				ScenarioExt::Global()->CurrentTint_Schemes.Green * 10,
				ScenarioExt::Global()->CurrentTint_Schemes.Blue * 10,
				false);
	}

	if (ScenarioExt::Global()->CurrentTint_Hashes != TintStruct { -1,-1,-1 })
	{
		ScenarioClass::UpdateHashPalLighting(
			ScenarioExt::Global()->CurrentTint_Hashes.Red * 10,
			ScenarioExt::Global()->CurrentTint_Hashes.Green * 10,
			ScenarioExt::Global()->CurrentTint_Hashes.Blue * 10,
			false);
	}

	ScenarioExt::RecreateLightSources();
	ScenarioClass::UpdateCellLighting();

	HouseClass::CurrentPlayer->RecheckRadar = true;

	return 0x6851B1;
}
