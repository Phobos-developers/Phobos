#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Techno/Body.h>

#include <functional>
#include <vector>

// Position shifting schedule
struct ShiftSchedule
{
public:
	struct Sample
	{
		CoordStruct Position;
		DirStruct Facing;
		float Pitch; // forward/back tilt in degrees
		float Roll;  // sideways tilt in degrees
		bool Finished;

		Sample()
			: Position { }
			, Facing { }
			, Pitch { 0.0f }
			, Roll { 0.0f }
			, Finished { false }
		{ }

		Sample(CoordStruct position, DirStruct facing, float pitch = 0.0f, float roll = 0.0f, bool finished = false)
		{
			Position = position;
			Facing = facing;
			Pitch = pitch;
			Roll = roll;
			Finished = finished;
		}
	};

	typedef std::function<void(TechnoClass* pThis, TechnoClass* pApplier, HouseClass* pApplierHouse)> ShiftProcessCallback;
	typedef std::function<FireError(TechnoClass* pThis)> ShiftCanFireCallback;

	ShiftSchedule() noexcept = default;

	ShiftSchedule(const Sample& start, const Sample& end,
		const std::vector<ShiftProcessCallback>& beginCallbacks = {},
		const std::vector<ShiftProcessCallback>& duringCallbacks = {},
		const std::vector<ShiftProcessCallback>& finishCallbacks = {},
		const ShiftCanFireCallback canFireCallback = nullptr) noexcept
		: Start(start)
		, End(end)
		, BeginCallbacks(beginCallbacks)
		, DuringCallbacks(duringCallbacks)
		, FinishCallbacks(finishCallbacks)
		, CanFireCallback(canFireCallback)
	{ }

	virtual ~ShiftSchedule() = default;

	// Decide the location and other status of the techno by frame.
	virtual Sample SampleAt(int elapsedFrames) const { return Sample(); };

	// Process callbacks.
	void BeginShiftProcess(TechnoClass* pThis);
	void DuringShiftProcess(TechnoClass* pThis);
	void FinishShiftProcess(TechnoClass* pThis);

	std::vector<ShiftProcessCallback> BeginCallbacks;
	std::vector<ShiftProcessCallback> DuringCallbacks;
	std::vector<ShiftProcessCallback> FinishCallbacks;
	ShiftCanFireCallback CanFireCallback;

	Sample Start {};
	Sample End {};
};

// Instant schedule: teleport — moves to destination on the next frame (elapsed >= 1)
class InstantShiftSchedule : public ShiftSchedule
{
public:
	InstantShiftSchedule() noexcept = default;
	virtual ~InstantShiftSchedule() override = default;

	InstantShiftSchedule(const Sample& start, const Sample& end,
		void* params = nullptr,
		const std::vector<ShiftProcessCallback>& beginCallbacks = {},
		const std::vector<ShiftProcessCallback>& finishCallbacks = {}) noexcept;

	Sample SampleAt(int elapsedFrames) const override;
};

struct LinearParams
{
	int Speed;
};

class LinearShiftSchedule : public ShiftSchedule
{
public:
	LinearShiftSchedule() noexcept = default;
	virtual ~LinearShiftSchedule() override = default;

	LinearShiftSchedule(const Sample& start, const Sample& end,
		void* params = nullptr,
		const std::vector<ShiftProcessCallback>& beginCallbacks = {},
		const std::vector<ShiftProcessCallback>& duringCallbacks = {},
		const std::vector<ShiftProcessCallback>& finishCallbacks = {},
		const ShiftCanFireCallback canFireCallback = nullptr) noexcept;

	Sample SampleAt(int elapsedFrames) const override;

private:
	int Speed { 0 }; // leptons / frame
};

struct ParabolaParams
{
public:

	double InitialAngle;     // degrees
	int InitialHorizSpeed;   // leptons / frame (horizontal component, constant during flight)

	ParabolaParams(double initialAngleDeg, int initialHorizSpeed)
	{
		InitialAngle = initialAngleDeg;
		InitialHorizSpeed = initialHorizSpeed;
	}
};

class ParabolaShiftSchedule : public ShiftSchedule
{
public:
	ParabolaShiftSchedule() noexcept = default;
	virtual ~ParabolaShiftSchedule() override = default;

	ParabolaShiftSchedule(const Sample& start, const Sample& end,
		void* params = nullptr,
		const std::vector<ShiftProcessCallback>& beginCallbacks = {},
		const std::vector<ShiftProcessCallback>& duringCallbacks = {},
		const std::vector<ShiftProcessCallback>& finishCallbacks = {},
		const ShiftCanFireCallback canFireCallback = nullptr) noexcept;

	Sample SampleAt(int elapsedFrames) const override;

private:
	int DurationFrames { 0 };
	double InitialAngleDeg { 0.0 };
	int InitialHorizSpeed { 0 };

	double Gravity { 1.0 };
};

struct PathParams
{
public:
	std::vector<int> PathDirections; // The path to follow
	int Speed; // leptons / frame
	int Height;

	PathParams() : Speed(0), Height(0) { }
	PathParams(const std::vector<int>& pathDirections, int speed, int height)
		: PathDirections(pathDirections), Speed(speed), Height(height) { }
};

class PathShiftSchedule : public ShiftSchedule
{
public:
	PathShiftSchedule() noexcept = default;
	virtual ~PathShiftSchedule() override = default;

	PathShiftSchedule(const Sample& start, const Sample& end,
		void* params = nullptr,
		const std::vector<ShiftProcessCallback>& beginCallbacks = {},
		const std::vector<ShiftProcessCallback>& duringCallbacks = {},
		const std::vector<ShiftProcessCallback>& finishCallbacks = {},
		const ShiftCanFireCallback canFireCallback = nullptr) noexcept;

	Sample SampleAt(int elapsedFrames) const override;

private:
	std::vector<int> PathDirections { };
	int Speed { 0 }; // leptons / frame
	int Height { 0 };
	std::vector<CoordStruct> PathCoords { };
};
