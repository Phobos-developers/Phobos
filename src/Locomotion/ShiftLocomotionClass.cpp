#include "ShiftLocomotionClass.h"

#include <Utilities/Debug.h>
#include <CellClass.h>
#include <TechnoClass.h>
#include <cmath>
#include <limits>
#include <comip.h>
#include <comdef.h>
#include <JumpjetLocomotionClass.h>

#pragma region Helpers

enum TrackerType
{
	Air,
	Ground,
	Underground
};

TrackerType GetTrackerTypeForHeight(int height)
{
	if (height >= 208)
		return TrackerType::Air;
	else if (height >= 0)
		return TrackerType::Ground;
	else
		return TrackerType::Underground;
}

bool ShiftLocomotionClass::IsAirLoco(ILocomotion* pLoco)
{
	return pLoco->Is_Moving() && (locomotion_cast<FlyLocomotionClass*>(pLoco) || locomotion_cast<JumpjetLocomotionClass*>(pLoco));
}

CoordStruct ShiftLocomotionClass::FindShiftDestination(FootClass* pTechno, CoordStruct idealDest, double searchRange, bool pathReachable)
{
	if (IsAirLoco(pTechno->Locomotor))
		return idealDest;

	auto checkMapCrd = [&](CellStruct mapCrd)
		{
			auto pCell = MapClass::Instance.GetCellAt(mapCrd);
			bool clear = pTechno->IsCellOccupied(pCell, FacingType::None, -1, nullptr, true) == Move::OK;

			if (!clear)
				return false;

			auto currentMapCrd = pTechno->GetMapCoords();
			bool reachable = !pathReachable || AStarClass::Instance.AttemptPath(&currentMapCrd, &mapCrd, pTechno, pTechno->OnBridge, pCell->ContainsBridge()) != INT_MAX;

			if (!reachable)
				return false;

			return true;
		};

	auto cells = GeneralUtils::AdjacentCellsInRange((unsigned int)(std::floor(searchRange)));
	auto idealMapCrd = CellClass::Coord2Cell(idealDest);
	auto offset = idealDest - MapClass::Instance.GetCellAt(idealMapCrd)->GetCoordsWithBridge();

	double bestDistSq = std::numeric_limits<double>::max();
	std::vector<CellStruct> bestMapCrds;

	for (auto cell : cells)
	{
		auto mapCrd = idealMapCrd + cell;

		if (!checkMapCrd(mapCrd))
			continue;

		double distSq = (mapCrd - idealMapCrd).MagnitudeSquared();

		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;
			bestMapCrds.clear();
			bestMapCrds.push_back(mapCrd);
		}
		else if (distSq == bestDistSq)
		{
			bestMapCrds.push_back(mapCrd);
		}
	}

	if (bestMapCrds.empty())
	{
		return CoordStruct::Empty;
	}

	CellStruct finalMapCrd = bestMapCrds[ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(bestMapCrds.size() - 1))];

	return MapClass::Instance.GetCellAt(finalMapCrd)->GetCoordsWithBridge() + offset;
}

#pragma endregion

void ShiftLocomotionClass::BeginShift(std::unique_ptr<ShiftSchedule> schedule)
{
	if (!this->LinkedTo || !schedule)
		return;

	this->Schedule = std::move(schedule);
	this->Elapsed = 0;
	this->IsShifting = true;

	// Process
	{
		this->Schedule->BeginShiftProcess(this->LinkedTo);
	}

	// Remove from trackers
	{
		bool isOnMap = this->LinkedTo->IsOnMap;
		this->LinkedTo->IsOnMap = false;
		this->LinkedTo->Mark(MarkType::Up);
		this->LinkedTo->IsOnMap = isOnMap;
		auto frozenStill = this->LinkedTo->FrozenStill;
		this->LinkedTo->FrozenStill = false;
		this->LinkedTo->GetCell()->RemoveContent(this->LinkedTo, this->LinkedTo->OnBridge);
		this->LinkedTo->FrozenStill = frozenStill;

		AircraftTrackerClass::Instance.Remove(this->LinkedTo);
		ScenarioExt::Global()->UndergroundTracker.Remove(this->LinkedTo);
		//auto const ext = TechnoExt::ExtMap.Find(this->LinkedTo);
		//if (ext) ext->SpecialTracked = true;
	}

	// Occupy the target cell
	{
		this->LinkedTo->UnmarkAllOccupationBits(this->LinkedTo->GetCoords());

		auto finalSample = this->Schedule->End;
		CoordStruct dest = finalSample.Position;
		int destCellZ = MapClass::Instance.GetCellFloorHeight(dest);
		int destCellHeight = destCellZ - dest.Z;

		if (destCellHeight < 208 && destCellHeight >= 0)
		{
			this->LinkedTo->MarkAllOccupationBits(dest);
		}
	}
}

void ShiftLocomotionClass::FinishShift(bool normal)
{
	if (this->Schedule)
	{
		// Remove occupation bits
		{
			auto finalSample = this->Schedule->End;
			CoordStruct dest = finalSample.Position;
			int destCellZ = MapClass::Instance.GetCellFloorHeight(dest);
			int destCellHeight = destCellZ - dest.Z;
			if (destCellHeight < 208 && destCellHeight >= 0)
			{
				this->LinkedTo->UnmarkAllOccupationBits(dest);
			}
		}

		// Add to trackers
		if (normal)
		{
			auto sample = this->Schedule->End;
			auto oldMapCrd = this->LinkedTo->GetMapCoords();
			auto oldHeight = this->LinkedTo->GetHeight();
			auto oldTrackerType = GetTrackerTypeForHeight(oldHeight);
			auto oldFlightMapCrd = this->LinkedTo->GetLastFlightMapCoords();
			bool oldAlt = this->LinkedTo->OnBridge;

			bool isOnMap = this->LinkedTo->IsOnMap;
			this->LinkedTo->IsOnMap = false;
			this->LinkedTo->SetLocation(sample.Position);
			this->LinkedTo->IsOnMap = isOnMap;
			this->LinkedTo->OnBridge = this->LinkedTo->GetCell()->ContainsBridge() && this->LinkedTo->Location.Z >= MapClass::Instance.GetCellFloorHeight(this->LinkedTo->Location) + CellClass::BridgeHeight;

			this->LinkedTo->PrimaryFacing.SetCurrent(sample.Facing);
			auto newMapCrd = this->LinkedTo->GetMapCoords();
			auto newHeight = this->LinkedTo->GetHeight();
			auto newTrackerType = GetTrackerTypeForHeight(newHeight);
			bool newAlt = this->LinkedTo->OnBridge;

			if (oldTrackerType != newTrackerType)
			{
				switch (oldTrackerType)
				{
				case TrackerType::Air:
					AircraftTrackerClass::Instance.Remove(this->LinkedTo);
					break;
				case TrackerType::Ground:
				{
					auto oldCell = MapClass::Instance.GetCellAt(oldMapCrd);
					auto frozenStill = this->LinkedTo->FrozenStill;
					this->LinkedTo->FrozenStill = false;
					oldCell->RemoveContent(this->LinkedTo, oldAlt);
					this->LinkedTo->FrozenStill = frozenStill;
					break;
				}
				case TrackerType::Underground:
					ScenarioExt::Global()->UndergroundTracker.Remove(this->LinkedTo);
					TechnoExt::ExtMap.Find(this->LinkedTo)->UndergroundTracked = false;
					break;
				default:
					break;
				}
				switch (newTrackerType)
				{
				case TrackerType::Air:
					AircraftTrackerClass::Instance.Add(this->LinkedTo);
					break;
				case TrackerType::Ground:
				{
					auto cell = MapClass::Instance.GetCellAt(newMapCrd);
					auto frozenStill = this->LinkedTo->FrozenStill;
					this->LinkedTo->FrozenStill = false;
					cell->AddContent(this->LinkedTo, newAlt);
					this->LinkedTo->FrozenStill = frozenStill;
				}
					break;
				case TrackerType::Underground:
					ScenarioExt::Global()->UndergroundTracker.AddUnique(this->LinkedTo);
					TechnoExt::ExtMap.Find(this->LinkedTo)->UndergroundTracked = true;
					break;
				default:
					break;
				}
			}
			else
			{
				switch (newTrackerType)
				{
				case Air:
					if (oldFlightMapCrd != newMapCrd)
					{
						AircraftTrackerClass::Instance.Update(this->LinkedTo, oldFlightMapCrd, newMapCrd);
					}
					break;
				case Ground:
					if (oldMapCrd != newMapCrd)
					{
						auto oldCell = MapClass::Instance.GetCellAt(oldMapCrd);
						auto newCell = MapClass::Instance.GetCellAt(newMapCrd);
						auto frozenStill = this->LinkedTo->FrozenStill;
						this->LinkedTo->FrozenStill = false;
						oldCell->RemoveContent(this->LinkedTo, oldAlt);
						this->LinkedTo->FrozenStill = frozenStill;
						frozenStill = this->LinkedTo->FrozenStill;
						this->LinkedTo->FrozenStill = false;
						newCell->AddContent(this->LinkedTo, newAlt);
						this->LinkedTo->FrozenStill = frozenStill;
					}
					break;
				case Underground:
					break;
				default:
					break;
				}
			}

			if (newTrackerType == TrackerType::Ground)
			{
				this->LinkedTo->UpdatePosition(PCPType::End);
			}
			else if (newTrackerType == TrackerType::Air)
			{
				if (this->Piggybacker && IsAirLoco(this->Piggybacker))
					this->LinkedTo->OnBridge = false; // Air units is always marked as not on bridge.
			}

			//auto const ext = TechnoExt::ExtMap.Find(this->LinkedTo);
			//if (ext) ext->SpecialTracked = false;
		}
		else
		{
			auto oldMapCrd = this->LinkedTo->GetMapCoords();
			auto oldHeight = this->LinkedTo->GetHeight();
			auto oldTrackerType = GetTrackerTypeForHeight(oldHeight);
			auto oldFlightMapCrd = this->LinkedTo->GetLastFlightMapCoords();
			bool oldAlt = this->LinkedTo->OnBridge;

			switch (oldTrackerType)
			{
			case TrackerType::Air:
				AircraftTrackerClass::Instance.Remove(this->LinkedTo);
				break;
			case TrackerType::Ground:
			{
				auto oldCell = MapClass::Instance.GetCellAt(oldMapCrd);
				auto frozenStill = this->LinkedTo->FrozenStill;
				this->LinkedTo->FrozenStill = false;
				oldCell->RemoveContent(this->LinkedTo, oldAlt);
				this->LinkedTo->FrozenStill = frozenStill;
				break;
			}
			case TrackerType::Underground:
				ScenarioExt::Global()->UndergroundTracker.Remove(this->LinkedTo);
				TechnoExt::ExtMap.Find(this->LinkedTo)->UndergroundTracked = false;
				break;
			default:
				break;
			}
		}

		// Process
		{
			this->Schedule->FinishShiftProcess(this->LinkedTo);
		}

		// End piggyback
		{
			auto pExt = TechnoExt::ExtMap.Find(this->LinkedTo);
			pExt->ShiftApplier = nullptr;
			pExt->ShiftApplierHouse = nullptr;
			this->LinkedTo->EnterIdleMode(false, false);
		}
	}

	this->IsShifting = false;
	this->Elapsed = 0;
}

bool ShiftLocomotionClass::Is_Moving()
{
	return this->IsShifting;
}

CoordStruct ShiftLocomotionClass::Destination()
{
	if (this->Schedule)
	{
		return this->Schedule->End.Position;
	}
	return this->LinkedTo ? this->LinkedTo->GetCenterCoords() : CoordStruct::Empty;
}

CoordStruct ShiftLocomotionClass::Head_To_Coord()
{
	if (this->Schedule)
	{
		return this->Schedule->End.Position;
	}
	return this->LinkedTo ? this->LinkedTo->GetCenterCoords() : CoordStruct::Empty;
}

bool ShiftLocomotionClass::Process()
{
	if (!this->IsShifting || !this->LinkedTo || !this->Schedule)
		return false;

	auto sample = this->Schedule->SampleAt(this->Elapsed);

	// Move to a location outside of the map is not allowed could lead to Fatal Errors.
	// Might need more investigation.
	if (!MapClass::Instance.IsWithinUsableArea(sample.Position))
	{
		this->FinishShift(false);
		this->LinkedTo->ReceiveDamage(&(this->LinkedTo->Health), 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);
		return false;
	}

	auto oldMapCrd = this->LinkedTo->GetMapCoords();
	auto oldHeight = this->LinkedTo->GetHeight();
	auto oldTrackerType = GetTrackerTypeForHeight(oldHeight);
	auto oldFlightMapCrd = this->LinkedTo->GetLastFlightMapCoords();
	bool oldAlt = this->LinkedTo->OnBridge;

	bool isOnMap = this->LinkedTo->IsOnMap;
	this->LinkedTo->IsOnMap = false;
	this->LinkedTo->SetLocation(sample.Position);
	this->LinkedTo->IsOnMap = isOnMap;
	this->LinkedTo->OnBridge = this->LinkedTo->GetCell()->ContainsBridge() && this->LinkedTo->Location.Z >= MapClass::Instance.GetCellFloorHeight(this->LinkedTo->Location) + CellClass::BridgeHeight;

	this->LinkedTo->PrimaryFacing.SetCurrent(sample.Facing);
	auto newMapCrd = this->LinkedTo->GetMapCoords();
	auto newHeight = this->LinkedTo->GetHeight();
	auto newTrackerType = GetTrackerTypeForHeight(newHeight);
	bool newAlt = this->LinkedTo->OnBridge;

	if (oldTrackerType != newTrackerType)
	{
		switch (oldTrackerType)
		{
		case TrackerType::Air:
			AircraftTrackerClass::Instance.Remove(this->LinkedTo);
			break;
		case TrackerType::Ground:
		{
			auto oldCell = MapClass::Instance.GetCellAt(oldMapCrd);
			auto frozenStill = this->LinkedTo->FrozenStill;
			this->LinkedTo->FrozenStill = false;
			oldCell->RemoveContent(this->LinkedTo, oldAlt);
			this->LinkedTo->FrozenStill = frozenStill;
			break;
		}
		case TrackerType::Underground:
			ScenarioExt::Global()->UndergroundTracker.Remove(this->LinkedTo);
			TechnoExt::ExtMap.Find(this->LinkedTo)->UndergroundTracked = false;
			break;
		default:
			break;
		}
		switch (newTrackerType)
		{
		case TrackerType::Air:
			AircraftTrackerClass::Instance.Add(this->LinkedTo);
			break;
		case TrackerType::Ground:
		{
			auto cell = MapClass::Instance.GetCellAt(newMapCrd);
			auto frozenStill = this->LinkedTo->FrozenStill;
			this->LinkedTo->FrozenStill = false;
			cell->AddContent(this->LinkedTo, newAlt);
			this->LinkedTo->FrozenStill = frozenStill;
		}
			break;
		case TrackerType::Underground:
			ScenarioExt::Global()->UndergroundTracker.AddUnique(this->LinkedTo);
			TechnoExt::ExtMap.Find(this->LinkedTo)->UndergroundTracked = true;
			break;
		default:
			break;
		}
	}
	else
	{
		switch (newTrackerType)
		{
		case Air:
			if (oldFlightMapCrd != newMapCrd)
			{
				AircraftTrackerClass::Instance.Update(this->LinkedTo, oldFlightMapCrd, newMapCrd);
			}
			break;
		case Ground:
			if (oldMapCrd != newMapCrd)
			{
				auto oldCell = MapClass::Instance.GetCellAt(oldMapCrd);
				auto newCell = MapClass::Instance.GetCellAt(newMapCrd);
				auto frozenStill = this->LinkedTo->FrozenStill;
				this->LinkedTo->FrozenStill = false;
				oldCell->RemoveContent(this->LinkedTo, oldAlt);
				this->LinkedTo->FrozenStill = frozenStill;
				frozenStill = this->LinkedTo->FrozenStill;
				this->LinkedTo->FrozenStill = false;
				newCell->AddContent(this->LinkedTo, newAlt);
				this->LinkedTo->FrozenStill = frozenStill;
			}
			break;
		case Underground:
			break;
		default:
			break;
		}
	}

	this->Schedule->DuringShiftProcess(this->LinkedTo);

	this->Elapsed++;

	if (sample.Finished)
	{
		this->FinishShift(true);
		return false;
	}

	return true;
}

void _stdcall ShiftLocomotionClass::Move_To(CoordStruct /*to*/)
{
	if (this->Schedule && this->Schedule->End.Position != CoordStruct::Empty)
		this->IsShifting = true;
}

HRESULT __stdcall ShiftLocomotionClass::Link_To_Object(void* pointer)
{
	auto result = LocomotionClass::Link_To_Object(pointer);

	if (SUCCEEDED(result))
	{
		auto ext = TechnoExt::ExtMap.Find(this->LinkedTo);

		if (ext && ext->QueuedShift)
		{
			BeginShift(std::move(ext->QueuedShift));
			ext->QueuedShift = nullptr;
		}
		else
		{
			Debug::Log("ShiftLocomotionClass linked to object without queued shift schedule.");
		}
	}

	return result;
}

// IPiggyback
HRESULT __stdcall ShiftLocomotionClass::Begin_Piggyback(ILocomotion* pointer)
{
	if (!pointer)
		return E_POINTER;

	if (this->Piggybacker)
		return E_FAIL;

	pointer->Mark_All_Occupation_Bits(MarkType::Up);

	this->Piggybacker = pointer;
	pointer->AddRef();

	return S_OK;
}

HRESULT __stdcall ShiftLocomotionClass::End_Piggyback(ILocomotion** pointer)
{
	if (!pointer)
		return E_POINTER;

	if (!this->Piggybacker)
		return S_FALSE;

	*pointer = this->Piggybacker.Detach();

	const auto pLinkedTo = this->LinkedTo;

	if (!pLinkedTo->Deactivated && !pLinkedTo->IsUnderEMP())
		this->Power_On();
	else
		this->Power_Off();

	// Handle HeadToCrd
	{
		(*pointer)->Force_Immediate_Destination(CoordStruct::Empty); // For mech and walk

		if ((*pointer)->Is_Moving())
		{
			auto oldHeadToCrd = (*pointer)->Head_To_Coord();
			auto offset = oldHeadToCrd - this->Schedule->Start.Position;
			(*pointer)->Force_Track((*pointer)->Get_Track_Number(), this->Schedule->End.Position + offset); // For drive and ship
		}

		if (auto dest = pLinkedTo->Destination)
		{
			pLinkedTo->SetDestination(nullptr, false);
			pLinkedTo->SetDestination(dest, false);
		}
	}

	return S_OK;
}

bool __stdcall ShiftLocomotionClass::Is_Ok_To_End()
{
	return !this->Is_Moving() && this->Piggybacker;
}

HRESULT __stdcall ShiftLocomotionClass::Piggyback_CLSID(GUID* classid)
{
	if (classid == nullptr)
		return E_POINTER;

	if (this->Piggybacker)
	{
		IPersistStreamPtr piggyAsPersist(this->Piggybacker);
		return piggyAsPersist->GetClassID(classid);
	}

	if (reinterpret_cast<IPiggyback*>(this) == nullptr)
		return E_FAIL;

	IPersistStreamPtr thisAsPersist(this);

	if (thisAsPersist == nullptr)
		return E_FAIL;

	return thisAsPersist->GetClassID(classid);
}

bool __stdcall ShiftLocomotionClass::Is_Piggybacking()
{
	return this->Piggybacker != nullptr;
}

static ShiftSchedule::Sample MakeSampleFromObject(FootClass* obj)
{
	ShiftSchedule::Sample s;
	if (obj)
	{
		s.Position = obj->GetCoords();
		s.Facing = obj->PrimaryFacing.Current();
		s.Pitch = 0.0f;
		s.Roll = 0.0f;
		s.Finished = false;
	}
	else
	{
		s.Position = CoordStruct::Empty;
		s.Facing = DirStruct{ DirType::North };
		s.Pitch = 0.0f;
		s.Roll = 0.0f;
		s.Finished = false;
	}
	return s;
}

Layer ShiftLocomotionClass::In_Which_Layer()
{
	auto height = this->LinkedTo->GetHeight();

	if (height >= 208)
		return Layer::Air;
	else if (height >= 0)
		return Layer::Ground;
	else
		return Layer::Underground;
}

FireError ShiftLocomotionClass::Can_Fire()
{
	if (this->Schedule)
	{
		if (auto cb = this->Schedule->CanFireCallback)
			return cb(this->LinkedTo);
	}

	return FireError::OK;
}

void ShiftLocomotionClass::Mark_All_Occupation_Bits(MarkType mark)
{
	if (!this->LinkedTo)
		return;

	if (this->Schedule)
	{
		CoordStruct head = this->Schedule->End.Position;

		if (mark != MarkType::Up)
			this->LinkedTo->MarkAllOccupationBits(head);
		else
			this->LinkedTo->UnmarkAllOccupationBits(head);
	}
}

void ShiftLocomotionClass::Limbo()
{
	this->FinishShift(false);
}
