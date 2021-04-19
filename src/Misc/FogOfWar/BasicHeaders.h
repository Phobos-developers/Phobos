#pragma once

#include <FoggedObjectClass.h>
#include <TerrainClass.h>
#include <OverlayClass.h>
#include <BuildingClass.h>
#include <FootClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <DisplayClass.h>
#include <SessionClass.h>
#include <HouseClass.h>
#include <RulesClass.h>
#include <GameModeOptionsClass.h>
#include <ScenarioClass.h>
#include <TacticalClass.h>

// STL
#include <vector>
#include <set>

#include "../../ExtraHeaders/IndexClass.h"

// Extra arrays
static constexpr constant_ptr<IndexClass<FoggedObjectClass*>, 0x8B3CC0> FogIndexes{};