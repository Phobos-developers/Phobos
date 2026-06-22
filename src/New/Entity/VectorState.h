#pragma once

#include <GeneralDefinitions.h>

#include <New/Type/AttachEffectTypeClass.h>

struct VectorState
{
	bool Initialized;
	int CurrentFrame;
	int DisabledTimer;
	double CurrentSpeed;
	double CurrentAngle;
	double CurrentCircleRadius;
	double CurrentCircleSpeed;
	double CurrentCircleAngle;
	CoordStruct InitialOriginPos;
	CoordStruct InitialLocation;
	CoordStruct PrevCirclePos;
	double ArcHeight;
	double ArcRotation;
	CoordStruct TargetOffset;
	double NormalRotF;
	double NormalRotL;
	double NormalRotH;
	double NormalStepF;
	double NormalStepL;
	double NormalStepH;
	int MovementFrames;
	double FacingRad;
	double TiltRad;
	CoordStruct OriginOffset;
	CoordStruct PrevCircleCenter;
	int OriginElapsed;
	double OriginSpeed;
	double OriginAngle;
	CoordStruct OriginTargetOffset;
	double OriginCircleRadiusRuntime;
	double OriginCircleSpeedRuntime;
	double OriginCircleAngleRuntime;
	double OriginNormalRotFRuntime;
	double OriginNormalRotLRuntime;
	double OriginNormalRotHRuntime;
	double OriginNormalStepF;
	double OriginNormalStepL;
	double OriginNormalStepH;
	double OriginFacing;
	double OriginTilt;

	VectorState()
		: Initialized { false }
		, CurrentFrame { 0 }
		, DisabledTimer { 0 }
		, CurrentSpeed { 0.0 }
		, CurrentAngle { 0.0 }
		, CurrentCircleRadius { 0.0 }
		, CurrentCircleSpeed { 0.0 }
		, CurrentCircleAngle { 0.0 }
		, InitialOriginPos { CoordStruct::Empty }
		, InitialLocation { CoordStruct::Empty }
		, PrevCirclePos { CoordStruct::Empty }
		, ArcHeight { 0.0 }
		, ArcRotation { 0.0 }
		, TargetOffset { CoordStruct::Empty }
		, NormalRotF { 0.0 }
		, NormalRotL { 0.0 }
		, NormalRotH { 0.0 }
		, NormalStepF { 0.0 }
		, NormalStepL { 0.0 }
		, NormalStepH { 0.0 }
		, MovementFrames { 0 }
		, FacingRad { 0.0 }
		, TiltRad { 0.0 }
		, OriginOffset { CoordStruct::Empty }
		, PrevCircleCenter { CoordStruct::Empty }
		, OriginElapsed { 0 }
		, OriginSpeed { 0.0 }
		, OriginAngle { 0.0 }
		, OriginTargetOffset { CoordStruct::Empty }
		, OriginCircleRadiusRuntime { 0.0 }
		, OriginCircleSpeedRuntime { 0.0 }
		, OriginCircleAngleRuntime { 0.0 }
		, OriginNormalRotFRuntime { 0.0 }
		, OriginNormalRotLRuntime { 0.0 }
		, OriginNormalRotHRuntime { 0.0 }
		, OriginNormalStepF { 0.0 }
		, OriginNormalStepL { 0.0 }
		, OriginNormalStepH { 0.0 }
		, OriginFacing { 0.0 }
		, OriginTilt { 0.0 }
	{ }
};

void VectorAI_Run(ObjectClass* pObject, AttachEffectTypeClass* pType, VectorState& state, ObjectClass* pInvoker, bool isBullet);
