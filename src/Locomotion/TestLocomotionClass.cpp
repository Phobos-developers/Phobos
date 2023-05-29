#include "TestLocomotionClass.h"

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
	return LinkedTo->IsCellOccupied(MapClass::Instance->GetCellAt(cell), -1, -1, nullptr, false);
}

bool TestLocomotionClass::Process()
{
	if (IsMoving)
	{
		CoordStruct coord = DestinationCoord;

		/**
		 *  Rotate the object around the center coord.
		 */
		int radius = Unsorted::LeptonsPerCell*2;
		coord.X += radius * std::sin(Angle);
		coord.Y += radius * std::cos(Angle);
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

	Angle = 0;

	IsMoving = false;
}

void TestLocomotionClass::Do_Turn(DirStruct coord)
{
	LinkedTo->PrimaryFacing.SetCurrent(coord);
}

// void TestLocomotionClass::Unlimbo()
// {
// 	Force_New_Slope(LinkedTo->Get_Cell_Ptr()->Ramp);
// }

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

void TestLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
	CoordStruct headTo = Head_To_Coord();
	if (mark != 0)
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

void TestLocomotionClass::Clear_Coords()
{
	TestLocomotionClass::Stop_Moving();
}
