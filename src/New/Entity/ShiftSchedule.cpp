#include "ShiftSchedule.h"

void ShiftSchedule::BeginShiftProcess(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& cb : BeginCallbacks)
	{
		if (cb)
			cb(pThis, pExt->ShiftApplier, pExt->ShiftApplierHouse);
	}
}

void ShiftSchedule::DuringShiftProcess(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& cb : DuringCallbacks)
	{
		if (cb)
			cb(pThis, pExt->ShiftApplier, pExt->ShiftApplierHouse);
	}
}

void ShiftSchedule::FinishShiftProcess(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& cb : FinishCallbacks)
	{
		if (cb)
			cb(pThis, pExt->ShiftApplier, pExt->ShiftApplierHouse);
	}
}

InstantShiftSchedule::InstantShiftSchedule(const Sample& start, const Sample& end,
	void* /*params*/,
	const std::vector<ShiftProcessCallback>& beginCallbacks,
	const std::vector<ShiftProcessCallback>& finishCallbacks) noexcept
	: ShiftSchedule(start, end, beginCallbacks, finishCallbacks)
{
	// Nothing to do here.
}

ShiftSchedule::Sample InstantShiftSchedule::SampleAt(int elapsedFrames) const
{
	ShiftSchedule::Sample s;

	if (elapsedFrames <= 0)
	{
		s = Start;
		s.Finished = false;
		return s;
	}

	s = End;
	s.Finished = true;
	return s;
}

LinearShiftSchedule::LinearShiftSchedule(const Sample& start, const Sample& end,
	void* params,
	const std::vector<ShiftProcessCallback>& beginCallbacks,
	const std::vector<ShiftProcessCallback>& duringCallbacks,
	const std::vector<ShiftProcessCallback>& finishCallbacks,
	const ShiftCanFireCallback canFireCallback) noexcept
	: ShiftSchedule(start, end, beginCallbacks, duringCallbacks, finishCallbacks, canFireCallback)
{
	if (params)
	{
		auto p = static_cast<LinearParams*>(params);
		Speed = p->Speed;
	}
}

ShiftSchedule::Sample LinearShiftSchedule::SampleAt(int elapsedFrames) const
{
	ShiftSchedule::Sample s;

	if (Speed <= 0)
	{
		if (elapsedFrames <= 0)
			return Start;
		s = End;
		s.Finished = true;
		return s;
	}

	// Compute horizontal distance (XY plane)
	const double dx = static_cast<double>(End.Position.X - Start.Position.X);
	const double dy = static_cast<double>(End.Position.Y - Start.Position.Y);
	const double dist = std::hypot(dx, dy);

	// If distance is 0, only consider Z or complete immediately
	if (dist <= 0.0)
	{
		if (elapsedFrames <= 0)
			return Start;
		s = End;
		s.Finished = true;
		return s;
	}

	// Distance covered (leptons) is given by speed * elapsedFrames
	double covered = static_cast<double>(Speed) * static_cast<double>(elapsedFrames);
	double frac = covered / dist;
	if (frac >= 1.0)
	{
		s = End;
		s.Finished = true;
		return s;
	}

	// Use frac to interpolate
	auto lerp = [](int a, int b, double tt) -> int
		{
			return static_cast<int>(std::llround(a + (b - a) * tt));
		};

	s.Position.X = lerp(Start.Position.X, End.Position.X, frac);
	s.Position.Y = lerp(Start.Position.Y, End.Position.Y, frac);
	s.Position.Z = lerp(Start.Position.Z, End.Position.Z, frac);

	s.Facing = Start.Facing;

	s.Pitch = Start.Pitch + static_cast<float>((End.Pitch - Start.Pitch) * frac);
	s.Roll = Start.Roll + static_cast<float>((End.Roll - Start.Roll) * frac);

	s.Finished = false;
	return s;
}

ParabolaShiftSchedule::ParabolaShiftSchedule(const Sample& start, const Sample& end,
	void* params,
	const std::vector<ShiftProcessCallback>& beginCallbacks,
	const std::vector<ShiftProcessCallback>& duringCallbacks,
	const std::vector<ShiftProcessCallback>& finishCallbacks,
	const ShiftCanFireCallback canFireCallback) noexcept
	: ShiftSchedule(start, end, beginCallbacks, duringCallbacks, finishCallbacks, canFireCallback)
{
	if (params)
	{
		auto p = static_cast<ParabolaParams*>(params);
		InitialAngleDeg = p->InitialAngle;
		InitialHorizSpeed = p->InitialHorizSpeed;
	}
	else
	{
		InitialAngleDeg = 0.0;
		InitialHorizSpeed = 0;
	}

	const double angleRad = Math::deg2rad(InitialAngleDeg);
	const double v_h = static_cast<double>(InitialHorizSpeed);

	const double dx = static_cast<double>(End.Position.X - Start.Position.X);
	const double dy = static_cast<double>(End.Position.Y - Start.Position.Y);
	const double horizDist = std::hypot(dx, dy);

	DurationFrames = 0;
	Gravity = 1.0;

	if (v_h > 0.0 && horizDist > 0.0)
	{
		const double time = horizDist / v_h;
		DurationFrames = static_cast<int>(std::max(1, static_cast<int>(std::ceil(time))));
		const double v_v = v_h * std::tan(angleRad);
		const double z0 = static_cast<double>(Start.Position.Z);
		const double z1 = static_cast<double>(End.Position.Z);
		const double T = time;
		if (T > 0.0)
		{
			Gravity = 2.0 * (v_v * T + z0 - z1) / (T * T);
		}
	}
	else
	{
		if (InitialHorizSpeed != 0)
			DurationFrames = 1;
		else
			DurationFrames = 0;
		Gravity = 1.0;
	}
}

ShiftSchedule::Sample ParabolaShiftSchedule::SampleAt(int elapsedFrames) const
{
	ShiftSchedule::Sample s;

	if (DurationFrames <= 0)
	{
		if (elapsedFrames <= 0)
			return Start;
		return End;
	}

	if (elapsedFrames <= 0) elapsedFrames = 0;
	if (elapsedFrames >= DurationFrames) elapsedFrames = DurationFrames;

	double t = static_cast<double>(elapsedFrames);

	const double dx = static_cast<double>(End.Position.X - Start.Position.X);
	const double dy = static_cast<double>(End.Position.Y - Start.Position.Y);
	const double horizDist = std::hypot(dx, dy);
	double dirX = 0.0;
	double dirY = 0.0;
	if (horizDist > 0.0)
	{
		dirX = dx / horizDist;
		dirY = dy / horizDist;
	}

	const double angleRad = Math::deg2rad(InitialAngleDeg);
	const double v_h = static_cast<double>(InitialHorizSpeed);
	const double v_v = v_h * std::tan(angleRad);

	s.Position.X = static_cast<int>(std::llround(Start.Position.X + dirX * v_h * t));
	s.Position.Y = static_cast<int>(std::llround(Start.Position.Y + dirY * v_h * t));

	double z = static_cast<double>(Start.Position.Z) + v_v * t - 0.5 * Gravity * t * t;
	s.Position.Z = static_cast<int>(std::llround(z));

	s.Facing = Start.Facing;

	const double normalized = static_cast<double>(elapsedFrames) / static_cast<double>(DurationFrames);
	s.Pitch = Start.Pitch + static_cast<float>((End.Pitch - Start.Pitch) * normalized);
	s.Roll = Start.Roll + static_cast<float>((End.Roll - Start.Roll) * normalized);

	s.Finished = (elapsedFrames >= DurationFrames);
	return s;
}

PathShiftSchedule::PathShiftSchedule(const Sample& start, const Sample& end,
	void* params,
	const std::vector<ShiftProcessCallback>& beginCallbacks,
	const std::vector<ShiftProcessCallback>& duringCallbacks,
	const std::vector<ShiftProcessCallback>& finishCallbacks,
	const ShiftCanFireCallback canFireCallback) noexcept
	: ShiftSchedule(start, end, beginCallbacks, duringCallbacks, finishCallbacks, canFireCallback)
{
	if (params)
	{
		auto p = static_cast<PathParams*>(params);
		PathDirections = p->PathDirections;
		Speed = p->Speed;
		Height = p->Height;

		auto currentCrd = start.Position;

		for (auto dir : PathDirections)
		{
			auto offset = CellClass::Cell2Coord(CellSpread::GetNeighbourOffset(static_cast<size_t>(dir)));
			currentCrd += offset;
			PathCoords.push_back(currentCrd);
		}
	}
}

ShiftSchedule::Sample PathShiftSchedule::SampleAt(int elapsedFrames) const
{
	ShiftSchedule::Sample s;

	// If speed not set or no precomputed path, fallback to instant behavior
	if (Speed <= 0 || PathCoords.empty())
	{
		if (elapsedFrames <= 0)
			return Start;
		s = End;
		s.Finished = true;
		return s;
	}

	if (elapsedFrames <= 0)
		return Start;

	// Total distance along the path (sum of segment lengths)
	double totalPathLength = 0.0;
	{
		double prevX = static_cast<double>(Start.Position.X);
		double prevY = static_cast<double>(Start.Position.Y);
		for (const auto& node : PathCoords)
		{
			const double nx = static_cast<double>(node.X);
			const double ny = static_cast<double>(node.Y);
			totalPathLength += std::hypot(nx - prevX, ny - prevY);
			prevX = nx;
			prevY = ny;
		}

		// Add final segment from last path node to End.Position
		const double endX = static_cast<double>(End.Position.X);
		const double endY = static_cast<double>(End.Position.Y);
		totalPathLength += std::hypot(endX - prevX, endY - prevY);
	}

	// Distance covered along path in leptons
	double distanceCovered = static_cast<double>(Speed) * static_cast<double>(elapsedFrames);

	// If we've reached or exceeded the whole path
	if (distanceCovered >= totalPathLength)
	{
		s = End;
		s.Finished = true;
		return s;
	}

	// Walk segments to find current segment and ratio inside it
	double traversed = 0.0;
	double currX = static_cast<double>(Start.Position.X);
	double currY = static_cast<double>(Start.Position.Y);

	// iterate through path nodes, then final segment to End.Position
	for (size_t i = 0; i <= PathCoords.size(); ++i)
	{
		double nodeX;
		double nodeY;

		if (i < PathCoords.size())
		{
			const auto& node = PathCoords[i];
			nodeX = static_cast<double>(node.X);
			nodeY = static_cast<double>(node.Y);
		}
		else
		{
			// final segment target is End.Position
			nodeX = static_cast<double>(End.Position.X);
			nodeY = static_cast<double>(End.Position.Y);
		}

		const double segDx = nodeX - currX;
		const double segDy = nodeY - currY;
		const double segLen = std::hypot(segDx, segDy);

		if (segLen <= 0.0)
		{
			// zero-length segment, advance
			currX = nodeX;
			currY = nodeY;
			continue;
		}

		if (traversed + segLen >= distanceCovered)
		{
			const double remain = distanceCovered - traversed;
			const double ratio = segLen > 0.0 ? (remain / segLen) : 0.0;

			const double posX = currX + segDx * ratio;
			const double posY = currY + segDy * ratio;

			s.Position.X = static_cast<int>(std::llround(posX));
			s.Position.Y = static_cast<int>(std::llround(posY));
			// Preserve original Z behaviour (path computed in XY plane)
			s.Position.Z = Start.Position.Z;

			// Interpolate pitch/roll along full path proportionally
			const double normalized = distanceCovered / totalPathLength;
			s.Pitch = Start.Pitch + static_cast<float>((End.Pitch - Start.Pitch) * normalized);
			s.Roll = Start.Roll + static_cast<float>((End.Roll - Start.Roll) * normalized);

			s.Facing = Start.Facing;
			s.Finished = false;
			return s;
		}

		// advance to next segment
		traversed += segLen;
		currX = nodeX;
		currY = nodeY;
	}

	// Fallback: if something odd happened, return End
	s = End;
	s.Finished = true;
	return s;
}
