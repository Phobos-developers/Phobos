#include "TestLocomotionClass.h"

#ifdef CUSTOM_LOCO_EXAMPLE_ENABLED // Define functions

#include <CellSpread.h>
#include <ScenarioClass.h>

#include <ParticleSystemClass.h>
#include <ParticleSystemTypeClass.h>

#include <AnimClass.h>
#include <AircraftClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <cmath>


bool TestLocomotionClass::Is_Moving()
{
	return IsMoving;
}

CoordStruct TestLocomotionClass::Destination()
{
	if (IsMoving) {
		return DestinationCoord;
	}

	return CoordStruct::Empty;
}

CoordStruct TestLocomotionClass::Head_To_Coord()
{
	if (IsMoving)
		return HeadToCoord;

	return LinkedTo->GetCenterCoords();
}

Move TestLocomotionClass::Can_Enter_Cell(CellStruct cell)
{
	return LinkedTo->IsCellOccupied(MapClass::Instance->GetCellAt(cell), FacingType::None, -1, nullptr, false);
}

bool TestLocomotionClass::Process()
{
	if (IsMoving)
	{
		CoordStruct coord = DestinationCoord;

		// Rotate the object around the center coord
		int radius = Unsorted::LeptonsPerCell*2;
		coord.X += static_cast<int>(radius * Math::sin(Angle));
		coord.Y += static_cast<int>(radius * Math::cos(Angle));
		//coord.Z // No need to adjust the height of the object.

		// Pickup the object the game world before we set the new coord.
		LinkedTo->Mark(MarkType::Up);

		if (Can_Enter_Cell(CellClass::Coord2Cell(coord)) == Move::OK)
		{
			LinkedTo->SetLocation(coord);

			// Increase the angle, wrapping if full circle is complete.
			double scale = 360.0;
			Angle += Math::deg2rad(360.0) / scale;
			if (Angle > 360.0)
				Angle -= 360.0;
		}

		LinkedTo->Mark(MarkType::Down);
	}

	return Is_Moving();
}

void TestLocomotionClass::Move_To(CoordStruct to)
{
	DestinationCoord = to;

	IsMoving = HeadToCoord != CoordStruct::Empty
		|| DestinationCoord != CoordStruct::Empty;
}

void TestLocomotionClass::Stop_Moving()
{
	HeadToCoord = CoordStruct::Empty;
	DestinationCoord = CoordStruct::Empty;

	Angle = 0.0;

	IsMoving = false;
}

void TestLocomotionClass::Do_Turn(DirStruct coord)
{
	LinkedTo->PrimaryFacing.SetCurrent(coord);
}

Layer TestLocomotionClass::In_Which_Layer()
{
	return Layer::Ground;
}

void TestLocomotionClass::Force_Immediate_Destination(CoordStruct coord)
{
	DestinationCoord = coord;
}

bool TestLocomotionClass::Is_Moving_Now()
{
	if (LinkedTo->PrimaryFacing.IsRotating())
		return true;

	if (Is_Moving())
		return HeadToCoord != CoordStruct::Empty && Apparent_Speed() > 0;

	return false;
}

void TestLocomotionClass::Mark_All_Occupation_Bits(MarkType mark)
{
	CoordStruct headTo = Head_To_Coord();
	if (mark != MarkType::Up)
		LinkedTo->MarkAllOccupationBits(headTo);
	else
		LinkedTo->UnmarkAllOccupationBits(headTo);
}

bool TestLocomotionClass::Is_Moving_Here(CoordStruct to)
{
	CoordStruct headTo = Head_To_Coord();
	return CellClass::Coord2Cell(headTo) == CellClass::Coord2Cell(to)
		&& std::abs(headTo.Z - to.Z) <= Unsorted::CellHeight;
}

bool TestLocomotionClass::Is_Really_Moving_Now()
{
	return IsMoving;
}

void TestLocomotionClass::Limbo()
{
	this->Stop_Moving();
}

#ifdef CUSTOM_LOCO_EXAMPLE_PIGGYBACK // Define IPiggyback functions

HRESULT TestLocomotionClass::Begin_Piggyback(ILocomotion* pointer)
{
	if (!pointer)
		return E_POINTER;

	if (this->Piggybacker)
		return E_FAIL;

	this->Piggybacker = pointer;

	return S_OK;
}

HRESULT TestLocomotionClass::End_Piggyback(ILocomotion** pointer)
{
	if (!pointer)
		return E_POINTER;

	if (!this->Piggybacker)
		return S_FALSE;

	// since pointer is a dumb pointer, we don't need to call Release,
	// hence we use Detach, otherwise the locomotor gets trashed
	*pointer = this->Piggybacker.Detach();

	// in order to play nice with IsLocomotor warheads probably also should
	// handle IsAttackedByLocomotor etc. warheads here, but none of the vanilla
	// warheads do this (except JumpjetLocomotionClass::End_Piggyback)

	return S_OK;
}

bool TestLocomotionClass::Is_Ok_To_End()
{
	// Actually a confusing name, should return true only if the piggybacking should be ended.
#ifdef LOCO_TEST_WARHEADS // Do not stop piggybacking automatically
	// In order for the inflict locomotor warhead to work as expected
	// we don't want to end piggybacking automatically for this loco
	return false;
#else
	return this->Piggybacker && !this->LinkedTo->IsAttackedByLocomotor;
#endif
}

HRESULT TestLocomotionClass::Piggyback_CLSID(GUID* classid)
{
	HRESULT hr;

	if (classid == nullptr)
		return E_POINTER;

	if (this->Piggybacker)
	{
		IPersistStreamPtr piggyAsPersist(this->Piggybacker);

		hr = piggyAsPersist->GetClassID(classid);
	}
	else
	{
		if (reinterpret_cast<IPiggyback*>(this) == nullptr)
			return E_FAIL;

		IPersistStreamPtr thisAsPersist(this);

		if (thisAsPersist == nullptr)
			return E_FAIL;

		hr = thisAsPersist->GetClassID(classid);
	}

	return hr;
}

bool TestLocomotionClass::Is_Piggybacking()
{
	return this->Piggybacker != nullptr;
}

#endif //CUSTOM_LOCO_EXAMPLE_PIGGYBACK

#endif //CUSTOM_LOCO_EXAMPLE_ENABLED
