#include "VectorState.h"
#include "AttachEffectClass.h"
#include <Ext/Techno/Body.h>
#include <cmath>
#include <cstdio>

static void VecLog(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	FILE* f = fopen("vec_debug.log", "a");
	if (f) {
		vfprintf(f, fmt, args);
		fclose(f);
	}
	va_end(args);
}

static double V_Random(double min, double max)
{
	if (min >= max) return min;
	return min + (max - min) * static_cast<double>(std::rand()) / RAND_MAX;
}

static double V_Deg2Rad(double deg) { return deg * 3.14159265358979323846 / 180.0; }
static double V_Rad2Deg(double rad) { return rad * 180.0 / 3.14159265358979323846; }

static DirStruct V_Radians2Dir(double rad)
{
	return DirStruct(static_cast<short>(rad * 32768.0 / 3.14159265358979323846));
}

static DirStruct V_Point2Dir(CoordStruct from, CoordStruct to)
{
	double dx = to.X - from.X, dy = to.Y - from.Y;
	return V_Radians2Dir(std::atan2(dy, dx));
}

static CoordStruct V_FLHAbsoluteOffset(CoordStruct flh, DirStruct facing)
{
	return AttachEffectClass::GetFLHAbsoluteCoords(CoordStruct::Empty, flh, facing);
}

void VectorAI_Run(ObjectClass* pObject, AttachEffectTypeClass* pType, VectorState& s, ObjectClass* pInvoker, bool isBullet)
{
	if (!pObject)
		return;

	if (pObject->WhatAmI() == AbstractType::Building)
		return;

	if (s.DisabledTimer > 0)
	{
		s.DisabledTimer--;
		return;
	}

	bool skipFrame = pType->Vector_TimeStep > 1 && (s.CurrentFrame % pType->Vector_TimeStep != 0);
	s.CurrentFrame++;

	if (skipFrame)
		return;

	auto EffectiveOriginNoUpdate = [&]() -> bool {
		if (pType->Vector_OriginNoUpdate.isset())
			return pType->Vector_OriginNoUpdate.Get();
		bool hasCircle = pType->Vector_CircleRadius > 0 || pType->Vector_CircleSpeed != 0 || pType->Vector_CircleAnglePerStep > 0.0
			|| (pType->Vector_CircleRandomRadiusMax > pType->Vector_CircleRandomRadiusMin)
			|| (pType->Vector_CircleRandomAngleMax > pType->Vector_CircleRandomAngleMin);
		bool hasTargetMode = pType->Vector_TargetFLH.isset() || pType->Vector_ReachTarget;
		bool hasMoveTo = pType->Vector_MoveTo != CoordStruct::Empty;
		return (hasTargetMode || hasMoveTo) && !hasCircle;
	};

	s.MovementFrames++;
	s.NormalRotF += s.NormalStepF;
	s.NormalRotL += s.NormalStepL;
	s.NormalRotH += s.NormalStepH;

	auto GetPos = [&]() { return pObject->GetCoords(); };

	if (!s.Initialized)
	{
		s.Initialized = true;
		s.InitialOriginPos = GetPos();
		switch (pType->Vector_Origin)
		{
		case VectorOrigin::Launcher:
			if (pInvoker) s.InitialOriginPos = pInvoker->GetCoords();
			break;
		case VectorOrigin::Target:
			if (isBullet)
			{
				auto const pB = static_cast<BulletClass*>(pObject);
				s.InitialOriginPos = pB->TargetCoords;
			}
			break;
		}
		s.InitialLocation = GetPos();

		if (pType->Vector_NormalVector.isset()
			|| (pType->Vector_NormalRandomFMax > pType->Vector_NormalRandomFMin)
			|| (pType->Vector_NormalRandomLMax > pType->Vector_NormalRandomLMin)
			|| (pType->Vector_NormalRandomHMax > pType->Vector_NormalRandomHMin))
		{
			CoordStruct nv = pType->Vector_NormalVector.isset() ? pType->Vector_NormalVector.Get() : CoordStruct{};
			if (pType->Vector_NormalRandomFMax > pType->Vector_NormalRandomFMin)
				nv.X = V_Random(pType->Vector_NormalRandomFMin, pType->Vector_NormalRandomFMax);
			if (pType->Vector_NormalRandomLMax > pType->Vector_NormalRandomLMin)
				nv.Y = V_Random(pType->Vector_NormalRandomLMin, pType->Vector_NormalRandomLMax);
			if (pType->Vector_NormalRandomHMax > pType->Vector_NormalRandomHMin)
				nv.Z = V_Random(pType->Vector_NormalRandomHMin, pType->Vector_NormalRandomHMax);
			double lenXY = std::sqrt((double)nv.X * nv.X + nv.Y * nv.Y);
			s.FacingRad = lenXY > 1e-6 ? std::atan2(nv.X, nv.Y) : 0.0;
			s.TiltRad = lenXY > 1e-6 ? std::atan2(nv.Z, lenXY) : (nv.Z > 0 ? 3.1415926535 / 2.0 : -3.1415926535 / 2.0);
		}
		else
		{
			switch (pType->Vector_Origin)
			{
			case VectorOrigin::Launcher:
				if (pInvoker)
				{
					auto const pLT = static_cast<TechnoClass*>(pInvoker);
					s.FacingRad = pLT->TurretFacing().GetRadian<32>();
				}
				break;
			case VectorOrigin::Target:
			{
				CoordStruct selfPos = GetPos();
				CoordStruct targetPos = selfPos;
				if (isBullet)
				{
					auto const pB = static_cast<BulletClass*>(pObject);
					if (pB->Target)
						targetPos = pB->Target->GetCoords();
					else
						targetPos = pB->TargetCoords;
				}
				else
				{
					auto const pT = static_cast<TechnoClass*>(pObject);
					if (pT->Target)
						targetPos = pT->Target->GetCoords();
				}
				double dx = selfPos.X - targetPos.X;
				double dy = selfPos.Y - targetPos.Y;
				double dz = selfPos.Z - targetPos.Z;
				s.FacingRad = std::atan2(dy, dx);
				double lenXY = std::sqrt(dx * dx + dy * dy);
				s.TiltRad = (lenXY > 1e-6 && pType->Vector_AllowedTilt) ? std::atan2(dz, lenXY) : 0.0;
				break;
			}
			default:
				if (isBullet)
				{
					auto const pB = static_cast<BulletClass*>(pObject);
					double vx = pB->Velocity.X, vy = pB->Velocity.Y;
					s.FacingRad = (vx != 0 || vy != 0) ? std::atan2(vy, vx) : 0.0;
				}
				else
				{
					auto const pT = static_cast<TechnoClass*>(pObject);
					s.FacingRad = pT->TurretFacing().GetRadian<32>();
				}
				s.TiltRad = 0.0;
				break;
			}
		}
		s.OriginFacing = s.FacingRad;
		s.OriginTilt = s.TiltRad;

		double speed;
		if (pType->Vector_InitialSpeed >= 0)
			speed = static_cast<double>(pType->Vector_InitialSpeed);
		else if (isBullet)
			speed = static_cast<double>(static_cast<BulletClass*>(pObject)->Speed);
		else
		{
			auto const pT = static_cast<TechnoClass*>(pObject);
			TechnoTypeClass* pTT = pT->GetTechnoType();
			speed = pTT->JumpjetSpeed > 0 ? static_cast<double>(pTT->JumpjetSpeed) : static_cast<double>(pTT->Speed);
		}
		if (pType->Vector_RandomSpeedMin != pType->Vector_RandomSpeedMax)
			speed = V_Random(pType->Vector_RandomSpeedMin, pType->Vector_RandomSpeedMax);
		s.CurrentSpeed = speed;

		s.ArcHeight = static_cast<double>(pType->Vector_ArcHeight);
		if (pType->Vector_ArcRandomHeightMin != pType->Vector_ArcRandomHeightMax)
			s.ArcHeight = V_Random(pType->Vector_ArcRandomHeightMin, pType->Vector_ArcRandomHeightMax);
		s.ArcRotation = pType->Vector_ArcRotation;
		if (pType->Vector_ArcRandomRotationMin != pType->Vector_ArcRandomRotationMax)
			s.ArcRotation = V_Random(pType->Vector_ArcRandomRotationMin, pType->Vector_ArcRandomRotationMax);
		if (pType->Vector_ArcPeakRandomPercentMin > 0 && pType->Vector_ArcPeakRandomPercentMax > pType->Vector_ArcPeakRandomPercentMin)
			s.ArcPeakPercent = V_Random(pType->Vector_ArcPeakRandomPercentMin, pType->Vector_ArcPeakRandomPercentMax) / 100.0;
		else if (pType->Vector_ArcPeakRandomPercent > 0)
			s.ArcPeakPercent = pType->Vector_ArcPeakRandomPercent / 100.0;
		else
			s.ArcPeakPercent = pType->Vector_ArcPeakPercent / 100.0;
		if (s.ArcPeakPercent <= 0.0) s.ArcPeakPercent = 0.5;
		if (s.ArcPeakPercent >= 1.0) s.ArcPeakPercent = 0.5;

		if (pType->Vector_TargetFLH.isset())
		{
			CoordStruct offset = CoordStruct::Empty;
			if (pType->Vector_TargetOffsetFMin != pType->Vector_TargetOffsetFMax)
				offset.X = pType->Vector_TargetOffsetFMin + rand() % (pType->Vector_TargetOffsetFMax - pType->Vector_TargetOffsetFMin + 1);
			if (pType->Vector_TargetOffsetLMin != pType->Vector_TargetOffsetLMax)
				offset.Y = pType->Vector_TargetOffsetLMin + rand() % (pType->Vector_TargetOffsetLMax - pType->Vector_TargetOffsetLMin + 1);
			if (pType->Vector_TargetOffsetHMin != pType->Vector_TargetOffsetHMax)
				offset.Z = pType->Vector_TargetOffsetHMin + rand() % (pType->Vector_TargetOffsetHMax - pType->Vector_TargetOffsetHMin + 1);
			s.TargetOffset = offset;
		}

		if (pType->Vector_NormalVector.isset())
		{
			s.NormalRotF = 0.0; s.NormalRotL = 0.0; s.NormalRotH = 0.0;
			auto ns = [](double ps, double m1, double M1, double m2, double M2) {
				if (ps != 0.0) return ps;
				if (M2 > m2 && (rand() % 2)) return m2 + (rand() / (double)RAND_MAX) * (M2 - m2);
				return M1 > m1 ? m1 + (rand() / (double)RAND_MAX) * (M1 - m1) : 0.0;
			};
			s.NormalStepF = ns(pType->Vector_NormalFAnglePerStep, pType->Vector_NormalFAngleRMin, pType->Vector_NormalFAngleRMax, pType->Vector_NormalFAngleRMin2, pType->Vector_NormalFAngleRMax2);
			s.NormalStepL = ns(pType->Vector_NormalLAnglePerStep, pType->Vector_NormalLAngleRMin, pType->Vector_NormalLAngleRMax, pType->Vector_NormalLAngleRMin2, pType->Vector_NormalLAngleRMax2);
			s.NormalStepH = ns(pType->Vector_NormalHAnglePerStep, pType->Vector_NormalHAngleRMin, pType->Vector_NormalHAngleRMax, pType->Vector_NormalHAngleRMin2, pType->Vector_NormalHAngleRMax2);
		}

		if (pType->Vector_OriginNormalVector.isset())
		{
			auto ns = [](double ps, double m1, double M1, double m2, double M2) {
				if (ps != 0.0) return ps;
				if (M2 > m2 && (rand() % 2)) return m2 + (rand() / (double)RAND_MAX) * (M2 - m2);
				return M1 > m1 ? m1 + (rand() / (double)RAND_MAX) * (M1 - m1) : 0.0;
			};
			s.OriginNormalStepF = ns(pType->Vector_OriginNormalFAnglePerStep, pType->Vector_OriginNormalFAngleRMin, pType->Vector_OriginNormalFAngleRMax, pType->Vector_OriginNormalFAngleRMin2, pType->Vector_OriginNormalFAngleRMax2);
			s.OriginNormalStepL = ns(pType->Vector_OriginNormalLAnglePerStep, pType->Vector_OriginNormalLAngleRMin, pType->Vector_OriginNormalLAngleRMax, pType->Vector_OriginNormalLAngleRMin2, pType->Vector_OriginNormalLAngleRMax2);
			s.OriginNormalStepH = ns(pType->Vector_OriginNormalHAnglePerStep, pType->Vector_OriginNormalHAngleRMin, pType->Vector_OriginNormalHAngleRMax, pType->Vector_OriginNormalHAngleRMin2, pType->Vector_OriginNormalHAngleRMax2);
		}
		else
		{
			s.OriginNormalStepF = pType->Vector_OriginNormalFAnglePerStep;
			s.OriginNormalStepL = pType->Vector_OriginNormalLAnglePerStep;
			s.OriginNormalStepH = pType->Vector_OriginNormalHAnglePerStep;
		}
	}

	// === Dynamic facing ===
	double effectiveFacing = s.FacingRad + V_Deg2Rad(s.NormalRotH);
	double effectiveTilt = s.TiltRad + V_Deg2Rad(s.NormalRotL);
	DirStruct mainFacingDir = V_Radians2Dir(effectiveFacing);

	if (pType->Vector_OriginIsOnWorld)
	{
		mainFacingDir = DirStruct{};
		effectiveFacing = 0.0;
		effectiveTilt = 0.0;
	}

	bool hasNormal = pType->Vector_NormalVector.isset()
		|| (pType->Vector_NormalRandomFMax > pType->Vector_NormalRandomFMin)
		|| (pType->Vector_NormalRandomLMax > pType->Vector_NormalRandomLMin)
		|| (pType->Vector_NormalRandomHMax > pType->Vector_NormalRandomHMin);
	if (!	EffectiveOriginNoUpdate() && !hasNormal && !pType->Vector_OriginIsOnWorld)
	{
		switch (pType->Vector_Origin)
		{
		case VectorOrigin::Source:
			if (pInvoker)
			{
				mainFacingDir = V_Point2Dir(pInvoker->GetCoords(), GetPos());
				effectiveFacing = mainFacingDir.GetRadian<32>();
				if (pType->Vector_AllowedTilt)
				{
					double dx = GetPos().X - pInvoker->GetCoords().X;
					double dy = GetPos().Y - pInvoker->GetCoords().Y;
					double dz = GetPos().Z - pInvoker->GetCoords().Z;
					double lenXY = std::sqrt(dx * dx + dy * dy);
					effectiveTilt = (lenXY > 1e-6) ? std::atan2(dz, lenXY) : 0.0;
				}
			}
			break;
		case VectorOrigin::Target:
		{
			CoordStruct targetPos;
			if (isBullet)
			{
				auto const pB = static_cast<BulletClass*>(pObject);
				if (pB->Target)
					targetPos = pB->Target->GetCoords();
				else
					targetPos = pB->TargetCoords;
			}
			else
			{
				auto const pT = static_cast<TechnoClass*>(pObject);
				if (pT->Target)
					targetPos = pT->Target->GetCoords();
				else
					break;
			}
			mainFacingDir = V_Point2Dir(targetPos, GetPos());
			effectiveFacing = mainFacingDir.GetRadian<32>();
			if (pType->Vector_AllowedTilt)
			{
				double dx = GetPos().X - targetPos.X;
				double dy = GetPos().Y - targetPos.Y;
				double dz = GetPos().Z - targetPos.Z;
				double lenXY = std::sqrt(dx * dx + dy * dy);
				effectiveTilt = (lenXY > 1e-6) ? std::atan2(dz, lenXY) : 0.0;
			}
			break;
		}
		case VectorOrigin::Self:
			if (isBullet)
			{
				double vx = static_cast<BulletClass*>(pObject)->Velocity.X;
				double vy = static_cast<BulletClass*>(pObject)->Velocity.Y;
				effectiveFacing = (vx != 0 || vy != 0) ? std::atan2(vy, vx) : 0.0;
				mainFacingDir = V_Radians2Dir(effectiveFacing);
			}
			else
			{
				auto const pT = static_cast<TechnoClass*>(pObject);
				mainFacingDir = pType->Vector_OriginIsOnBody
					? pT->PrimaryFacing.Current()
					: pT->TurretFacing();
				effectiveFacing = mainFacingDir.GetRadian<32>();
			}
			break;
		case VectorOrigin::Launcher:
			if (pInvoker && pInvoker->IsAlive)
			{
				auto const pLT = static_cast<TechnoClass*>(pInvoker);
				mainFacingDir = pType->Vector_OriginIsOnBody
					? pLT->PrimaryFacing.Current()
					: pLT->TurretFacing();
				effectiveFacing = mainFacingDir.GetRadian<32>();
			}
			break;
		}
	}

	// === Origin coordinate (per-frame tracking) ===
	CoordStruct originPos = GetPos();

	switch (pType->Vector_Origin)
	{
	case VectorOrigin::Target:
		if (	EffectiveOriginNoUpdate())
			originPos = s.InitialOriginPos;
		else if (isBullet)
		{
			auto const pB = static_cast<BulletClass*>(pObject);
			originPos = pB->Target ? pB->Target->GetCoords() : pB->TargetCoords;
		}
		else
		{
			auto const pT = static_cast<TechnoClass*>(pObject);
			originPos = pT->Target ? pT->Target->GetCoords() : originPos;
		}
		break;
	case VectorOrigin::Launcher:
		originPos = 	EffectiveOriginNoUpdate() ? s.InitialOriginPos :
			(pInvoker ? pInvoker->GetCoords() : GetPos());
		break;
	case VectorOrigin::Source:
		originPos = 	EffectiveOriginNoUpdate() ? s.InitialOriginPos :
			(pInvoker ? pInvoker->GetCoords() : GetPos());
		break;
	case VectorOrigin::Self:
		originPos = 	EffectiveOriginNoUpdate() ? s.InitialOriginPos : GetPos();
		break;
	}

	if (pType->Vector_OriginFLH.isset() && pType->Vector_Origin != VectorOrigin::Self)
		originPos = AttachEffectClass::GetFLHAbsoluteCoords(originPos, pType->Vector_OriginFLH, mainFacingDir);

	CoordStruct currentPos = GetPos();

	// ================================================================
	// Circle mode
	// ================================================================
	bool hasCircle = pType->Vector_CircleRadius > 0 || pType->Vector_CircleSpeed != 0 || pType->Vector_CircleAnglePerStep > 0.0
		|| (pType->Vector_CircleRandomRadiusMax > pType->Vector_CircleRandomRadiusMin)
		|| (pType->Vector_CircleRandomAngleMax > pType->Vector_CircleRandomAngleMin)
		|| (pType->Vector_CircleRandomAngleMax2 > pType->Vector_CircleRandomAngleMin2);

	if (hasCircle)
	{
		double calcRadius = static_cast<double>(pType->Vector_CircleRadius);
		if (calcRadius <= 0.0)
		{
			double tdx = currentPos.X - originPos.X;
			double tdy = currentPos.Y - originPos.Y;
			calcRadius = std::sqrt(tdx * tdx + tdy * tdy);
		}

		if (s.MovementFrames == 1)
		{
			s.CurrentCircleSpeed = static_cast<double>(pType->Vector_CircleSpeed);
			s.CurrentCircleRadius = calcRadius;
			if (pType->Vector_CircleRandomRadiusMax > pType->Vector_CircleRandomRadiusMin)
				s.CurrentCircleRadius = V_Random(static_cast<double>(pType->Vector_CircleRandomRadiusMin), static_cast<double>(pType->Vector_CircleRandomRadiusMax));
		}
		s.CurrentCircleSpeed += pType->Vector_CircleSpeedAcceleration;
		if (pType->Vector_CircleMaxSpeed != 0 && s.CurrentCircleSpeed > pType->Vector_CircleMaxSpeed)
			s.CurrentCircleSpeed = static_cast<double>(pType->Vector_CircleMaxSpeed);
		if (pType->Vector_CircleMinSpeed != 0 && s.CurrentCircleSpeed < pType->Vector_CircleMinSpeed)
			s.CurrentCircleSpeed = static_cast<double>(pType->Vector_CircleMinSpeed);

		if (s.MovementFrames == 1)
		{
			s.CurrentCircleAngle = pType->Vector_CircleAnglePerStep;
			if (pType->Vector_CircleRandomAngleMax > pType->Vector_CircleRandomAngleMin)
			{
				bool useSecond = pType->Vector_CircleRandomAngleMax2 > pType->Vector_CircleRandomAngleMin2
					&& (std::rand() % 2);
				if (useSecond)
					s.CurrentCircleAngle = V_Random(pType->Vector_CircleRandomAngleMin2, pType->Vector_CircleRandomAngleMax2);
				else
					s.CurrentCircleAngle = V_Random(pType->Vector_CircleRandomAngleMin, pType->Vector_CircleRandomAngleMax);
			}
		}
		s.CurrentCircleAngle += pType->Vector_CircleAngleAcceleration;
		if (pType->Vector_CircleMaxAngle != 0 && s.CurrentCircleAngle > pType->Vector_CircleMaxAngle)
			s.CurrentCircleAngle = pType->Vector_CircleMaxAngle;
		if (pType->Vector_CircleMinAngle != 0 && s.CurrentCircleAngle < pType->Vector_CircleMinAngle)
			s.CurrentCircleAngle = pType->Vector_CircleMinAngle;

		double speed = s.CurrentCircleSpeed;
		double angleStep = s.CurrentCircleAngle;
		if (speed <= 0.0 && angleStep > 0.0)
			speed = calcRadius * V_Deg2Rad(angleStep);
		else if (angleStep <= 0.0 && speed > 0.0)
			angleStep = V_Rad2Deg(speed / calcRadius);

		CoordStruct circleCenter = originPos;
		if (pType->Vector_CircleOrigin.isset())
		{
			if (pType->Vector_AllowOriginTilt)
				circleCenter = AttachEffectClass::GetFLHAbsoluteCoords(originPos, pType->Vector_CircleOrigin, mainFacingDir);
			else
				circleCenter = { originPos.X + pType->Vector_CircleOrigin.Get().X, originPos.Y + pType->Vector_CircleOrigin.Get().Y, originPos.Z + pType->Vector_CircleOrigin.Get().Z };
		}

		bool hasOriginSub = pType->Vector_OriginMoveTo.isset()
			|| pType->Vector_OriginTargetFLH.isset()
			|| pType->Vector_OriginCircleRadius >= 0 || pType->Vector_OriginCircleSpeed != 0 || pType->Vector_OriginCircleAnglePerStep != 0;

		if (hasOriginSub)
		{
			VecLog("[ORIGIN] Frame=%d MOVFRM=%d CR=%d CSP=%d CAng=%.1f\n",
				Unsorted::CurrentFrame, s.MovementFrames,
				(int)pType->Vector_OriginCircleRadius, (int)pType->Vector_OriginCircleSpeed,
				pType->Vector_OriginCircleAnglePerStep);
			CoordStruct baseCenter = originPos;

			if (pType->Vector_OriginOrigin != VectorOrigin::Self)
			{
				switch (pType->Vector_OriginOrigin)
				{
				case VectorOrigin::Launcher:
					if (pInvoker) baseCenter = pInvoker->GetCoords();
					break;
				case VectorOrigin::Target:
					if (isBullet)
					{
						auto const pB = static_cast<BulletClass*>(pObject);
						if (pB->Target) baseCenter = pB->Target->GetCoords();
						else if (pB->Owner && pB->Owner->Target) baseCenter = pB->Owner->Target->GetCoords();
						else baseCenter = pB->TargetCoords;
					}
					else
					{
						auto const pT = static_cast<TechnoClass*>(pObject);
						if (pT->Target) baseCenter = pT->Target->GetCoords();
						else
						{
							auto const pFoot = abstract_cast<FootClass*>(pT);
							if (pFoot && pFoot->Destination) baseCenter = pFoot->Destination->GetCoords();
						}
					}
					break;
				case VectorOrigin::Source:
					if (pInvoker) baseCenter = pInvoker->GetCoords();
					break;
				}
			}
			else if (pType->Vector_OriginOriginFLH.isset())
			{
				baseCenter.X += pType->Vector_OriginOriginFLH.Get().X;
				baseCenter.Y += pType->Vector_OriginOriginFLH.Get().Y;
				baseCenter.Z += pType->Vector_OriginOriginFLH.Get().Z;
			}

			if (pType->Vector_OriginCircleOffset.isset())
			{
				baseCenter.X += pType->Vector_OriginCircleOffset.Get().X;
				baseCenter.Y += pType->Vector_OriginCircleOffset.Get().Y;
				baseCenter.Z += pType->Vector_OriginCircleOffset.Get().Z;
			}

			if (s.MovementFrames == 1)
			{
				s.OriginOffset = { circleCenter.X - baseCenter.X, circleCenter.Y - baseCenter.Y, circleCenter.Z - baseCenter.Z };
				s.OriginCircleRadiusRuntime = static_cast<double>(pType->Vector_OriginCircleRadius);
				s.OriginCircleSpeedRuntime = static_cast<double>(pType->Vector_OriginCircleSpeed);
				s.OriginCircleAngleRuntime = 0.0;
				{
					int ofx = pType->Vector_OriginTargetOffsetFMax > pType->Vector_OriginTargetOffsetFMin
						? static_cast<int>(V_Random(static_cast<double>(pType->Vector_OriginTargetOffsetFMin), static_cast<double>(pType->Vector_OriginTargetOffsetFMax))) : 0;
					int ofy = pType->Vector_OriginTargetOffsetLMax > pType->Vector_OriginTargetOffsetLMin
						? static_cast<int>(V_Random(static_cast<double>(pType->Vector_OriginTargetOffsetLMin), static_cast<double>(pType->Vector_OriginTargetOffsetLMax))) : 0;
					int ofz = pType->Vector_OriginTargetOffsetHMax > pType->Vector_OriginTargetOffsetHMin
						? static_cast<int>(V_Random(static_cast<double>(pType->Vector_OriginTargetOffsetHMin), static_cast<double>(pType->Vector_OriginTargetOffsetHMax))) : 0;
					s.OriginTargetOffset = { ofx, ofy, ofz };
				}

				if (pType->Vector_OriginNormalVector.isset())
				{
					CoordStruct nv = pType->Vector_OriginNormalVector.Get();
					double len = std::sqrt(static_cast<double>(nv.X * nv.X + nv.Y * nv.Y));
					s.OriginFacing = len > 1e-6 ? std::atan2(static_cast<double>(nv.X), static_cast<double>(nv.Y)) : 0;
					s.OriginTilt = len > 1e-6 ? std::atan2(static_cast<double>(nv.Z), len) : (nv.Z > 0 ? 3.14159265358979323846 / 2.0 : -3.14159265358979323846 / 2.0);
					// 哨兵：NormalVector 存在但未显式设 CircleRadius 时标记为 0
					if (pType->Vector_OriginCircleRadius < 0)
						s.OriginCircleRadiusRuntime = 0.0;
				}
				else
				{
					// 默认水平圆面（法向量朝上）
					s.OriginFacing = 0;
					s.OriginTilt = 3.14159265358979323846 / 2.0;
				}
			}

			s.OriginNormalRotFRuntime += s.OriginNormalStepF;
			s.OriginNormalRotLRuntime += s.OriginNormalStepL;
			s.OriginNormalRotHRuntime += s.OriginNormalStepH;

			double oFacing = s.OriginFacing + V_Deg2Rad(s.OriginNormalRotHRuntime);
			double oTilt = s.OriginTilt + V_Deg2Rad(s.OriginNormalRotLRuntime);

			CoordStruct originCenter = { baseCenter.X + s.OriginOffset.X, baseCenter.Y + s.OriginOffset.Y, baseCenter.Z + s.OriginOffset.Z };
			CoordStruct disp{ 0, 0, 0 };

			if (pType->Vector_OriginMoveTo.isset())
			{
				s.OriginAngle += pType->Vector_OriginCircleAnglePerStep;
				CoordStruct mto = pType->Vector_OriginMoveTo.Get();
				if (pType->Vector_OriginGrowRate.isset())
				{
					mto.X += pType->Vector_OriginGrowRate.Get().X * s.OriginElapsed;
					mto.Y += pType->Vector_OriginGrowRate.Get().Y * s.OriginElapsed;
					mto.Z += pType->Vector_OriginGrowRate.Get().Z * s.OriginElapsed;
				}
				disp = V_FLHAbsoluteOffset(mto, V_Radians2Dir(oFacing + V_Deg2Rad(s.OriginAngle)));
			}
			else if (pType->Vector_OriginTargetFLH.isset())
			{
				if (s.OriginElapsed == 0)
					s.OriginSpeed = pType->Vector_OriginInitialSpeed >= 0 ? pType->Vector_OriginInitialSpeed : 40.0;

				CoordStruct targetWorld = AttachEffectClass::GetFLHAbsoluteCoords(baseCenter, pType->Vector_OriginTargetFLH.Get() + s.OriginTargetOffset, V_Radians2Dir(oFacing));

				if (pType->Vector_OriginReachTarget)
				{
					int totalDuration = pType->Duration / pType->Vector_TimeStep;
					if (totalDuration < 1) totalDuration = 1;
					int effectiveDuration = totalDuration - pType->Vector_DisabledFrames;
					if (effectiveDuration < 1) effectiveDuration = 1;
					int effectiveSteps = effectiveDuration;
					int rem = effectiveSteps - s.MovementFrames;
					if (rem <= 0)
					{
						disp.X = targetWorld.X - originCenter.X;
						disp.Y = targetWorld.Y - originCenter.Y;
						disp.Z = targetWorld.Z - originCenter.Z;
						s.OriginOffset.X += disp.X; s.OriginOffset.Y += disp.Y; s.OriginOffset.Z += disp.Z;
						circleCenter = { baseCenter.X + s.OriginOffset.X, baseCenter.Y + s.OriginOffset.Y, baseCenter.Z + s.OriginOffset.Z };
						s.PrevCircleCenter = circleCenter;
						s.DisabledTimer = -1; // Deactivate-equivalent
						s.OriginElapsed = 0; // no further OriginUpdate
					}
					else
					{
						disp.X = (targetWorld.X - originCenter.X) / rem;
						disp.Y = (targetWorld.Y - originCenter.Y) / rem;
						disp.Z = (targetWorld.Z - originCenter.Z) / rem;
						if (pType->Vector_OriginArcHeight)
						{
							double t = effectiveSteps > 0 ? static_cast<double>(s.OriginElapsed) / effectiveSteps : 0;
							double t0 = effectiveSteps > 0 ? static_cast<double>(s.OriginElapsed - 1) / effectiveSteps : 0;
							disp.Z += static_cast<int>(4.0 * pType->Vector_OriginArcHeight * (t * (1 - t) - t0 * (1 - t0)));
						}
					}
				}
				else
				{
					s.OriginSpeed += static_cast<double>(pType->Vector_OriginAcceleration);
					if (pType->Vector_OriginMaxSpeed >= 0 && s.OriginSpeed > pType->Vector_OriginMaxSpeed)
						s.OriginSpeed = static_cast<double>(pType->Vector_OriginMaxSpeed);
					if (pType->Vector_OriginMinSpeed >= 0 && s.OriginSpeed < pType->Vector_OriginMinSpeed)
						s.OriginSpeed = static_cast<double>(pType->Vector_OriginMinSpeed);

					int dx = targetWorld.X - originCenter.X, dy = targetWorld.Y - originCenter.Y, dz = targetWorld.Z - originCenter.Z;
					double dist = std::sqrt(static_cast<double>(dx * dx + dy * dy + dz * dz));
					if (dist >= 1.0)
					{
						double sv = s.OriginSpeed / dist;
						disp = { static_cast<int>(dx * sv), static_cast<int>(dy * sv), static_cast<int>(dz * sv) };
					}
				}
			}
			else
			{
				s.OriginCircleRadiusRuntime += pType->Vector_OriginCircleRadiusGrow;
				double tr = s.OriginCircleRadiusRuntime;
				if (pType->Vector_OriginCircleMaxRadius > 0 && tr > pType->Vector_OriginCircleMaxRadius) tr = pType->Vector_OriginCircleMaxRadius;
				if (pType->Vector_OriginCircleMinRadius > 0 && tr < pType->Vector_OriginCircleMinRadius) tr = pType->Vector_OriginCircleMinRadius;
				double stepO = pType->Vector_OriginCircleAnglePerStep;
				if (pType->Vector_OriginCircleSpeed != 0 && tr > 0)
					stepO = V_Rad2Deg(pType->Vector_OriginCircleSpeed / tr);
				s.OriginCircleAngleRuntime += stepO;
				double r = pType->Vector_OriginLissajous ? V_Deg2Rad(s.OriginCircleAngleRuntime) : V_Deg2Rad(stepO);
				double ca = std::cos(r), sa = std::sin(r);
				double dxO = static_cast<double>(s.OriginOffset.X);
				double dyO = static_cast<double>(s.OriginOffset.Y);
				double dzO = static_cast<double>(s.OriginOffset.Z);
				double cf = std::cos(oFacing), sf = std::sin(oFacing), ct = std::cos(oTilt), st = std::sin(oTilt);
				double dL = dxO * (-sf) + dyO * cf;
				double dH = dxO * (-cf * st) + dyO * (-sf * st) + dzO * ct;
				double cd = std::sqrt(dL * dL + dH * dH);
				if (cd < 1.0 && tr > 0) { dL = tr; dH = 0; cd = tr; }
				else if (cd < 1.0) cd = 1.0;
				double rL = (dL / cd * tr * ca) - (dH / cd * tr * sa);
				double rH = (dL / cd * tr * sa) + (dH / cd * tr * ca);
				CoordStruct newOffset;
				newOffset.X = static_cast<int>(rL * (-sf) + rH * (-cf * st));
				newOffset.Y = static_cast<int>(rL * cf + rH * (-sf * st));
				newOffset.Z = static_cast<int>(rH * ct);
				disp.X = newOffset.X - s.OriginOffset.X;
				disp.Y = newOffset.Y - s.OriginOffset.Y;
				disp.Z = newOffset.Z - s.OriginOffset.Z;
			}

			s.OriginOffset.X += disp.X; s.OriginOffset.Y += disp.Y; s.OriginOffset.Z += disp.Z;
			circleCenter = { baseCenter.X + s.OriginOffset.X, baseCenter.Y + s.OriginOffset.Y, baseCenter.Z + s.OriginOffset.Z };
			VecLog("[ORIGIN] of=%.3f ot=%.3f rad=%.1f ang=%.1f disp=(%d,%d,%d) offset=(%d,%d,%d) center=(%d,%d,%d)\n",
				oFacing, oTilt, s.OriginCircleRadiusRuntime, s.OriginCircleAngleRuntime,
				disp.X, disp.Y, disp.Z,
				s.OriginOffset.X, s.OriginOffset.Y, s.OriginOffset.Z,
				circleCenter.X, circleCenter.Y, circleCenter.Z);
			s.OriginElapsed++;
		}

		CoordStruct centerDelta{ 0, 0, 0 };
		bool useCenterTracking = false;
		if (s.PrevCircleCenter.X || s.PrevCircleCenter.Y || s.PrevCircleCenter.Z)
		{
			centerDelta.X = circleCenter.X - s.PrevCircleCenter.X;
			centerDelta.Y = circleCenter.Y - s.PrevCircleCenter.Y;
			centerDelta.Z = circleCenter.Z - s.PrevCircleCenter.Z;
			useCenterTracking = (pType->Vector_OriginCircleRadius >= 0 || pType->Vector_OriginCircleSpeed != 0 || pType->Vector_OriginCircleAnglePerStep != 0);
		}
		s.PrevCircleCenter = circleCenter;

		CoordStruct trackPos = currentPos;
		if (useCenterTracking)
		{
			trackPos.X += centerDelta.X; trackPos.Y += centerDelta.Y; trackPos.Z += centerDelta.Z;
		}

		double dx = static_cast<double>(trackPos.X - circleCenter.X);
		double dy = static_cast<double>(trackPos.Y - circleCenter.Y);
		double dz = static_cast<double>(trackPos.Z - circleCenter.Z);
		double currentDist;
		bool useTiltPlane = hasNormal || (pType->Vector_AllowedTilt && effectiveTilt != 0.0);

		if (useTiltPlane)
		{
			double cosF = std::cos(effectiveFacing), sinF = std::sin(effectiveFacing);
			double cosT = std::cos(effectiveTilt), sinT = std::sin(effectiveTilt);
			double dL = dx * (-sinF) + dy * cosF;
			double dH = dx * (-cosF * sinT) + dy * (-sinF * sinT) + dz * cosT;
			currentDist = std::sqrt(dL * dL + dH * dH);
		}
		else
		{
			currentDist = std::sqrt(dx * dx + dy * dy);
		}
		bool startAtCenter = currentDist < 1.0;
		if (currentDist < 1.0) currentDist = 1.0;

		s.CurrentCircleRadius += pType->Vector_CircleRadiusGrow;
		double targetRadius = s.CurrentCircleRadius;
		if (pType->Vector_CircleMaxRadius > 0 && targetRadius > pType->Vector_CircleMaxRadius)
			targetRadius = static_cast<double>(pType->Vector_CircleMaxRadius);
		if (pType->Vector_CircleMinRadius > 0 && targetRadius < pType->Vector_CircleMinRadius)
			targetRadius = static_cast<double>(pType->Vector_CircleMinRadius);

		if (startAtCenter)
		{
			dx = targetRadius; dy = 0.0; currentDist = targetRadius;
		}

		double rad = V_Deg2Rad(angleStep);
		double cosA = std::cos(rad), sinA = std::sin(rad);
		CoordStruct moveDisp{ 0, 0, 0 };

		if (useTiltPlane)
		{
			double cosF = std::cos(effectiveFacing), sinF = std::sin(effectiveFacing);
			double cosT = std::cos(effectiveTilt), sinT = std::sin(effectiveTilt);
			double dL = dx * (-sinF) + dy * cosF;
			double dH = dx * (-cosF * sinT) + dy * (-sinF * sinT) + dz * cosT;
			double curDist = std::sqrt(dL * dL + dH * dH);
			if (curDist < 1.0) curDist = 1.0;
			double ndL = dL / curDist * targetRadius;
			double ndH = dH / curDist * targetRadius;
			double rL = ndL * cosA - ndH * sinA;
			double rH = ndL * sinA + ndH * cosA;
			moveDisp.X = circleCenter.X + static_cast<int>(rL * (-sinF) + rH * (-cosF * sinT)) - currentPos.X;
			moveDisp.Y = circleCenter.Y + static_cast<int>(rL * cosF + rH * (-sinF * sinT)) - currentPos.Y;
			moveDisp.Z = circleCenter.Z + static_cast<int>(rH * cosT) - currentPos.Z;
		}
		else
		{
			double ndx = dx / currentDist * targetRadius;
			double ndy = dy / currentDist * targetRadius;
			double rx = ndx * cosA - ndy * sinA;
			double ry = ndx * sinA + ndy * cosA;
			moveDisp.X = circleCenter.X + static_cast<int>(rx) - currentPos.X;
			moveDisp.Y = circleCenter.Y + static_cast<int>(ry) - currentPos.Y;
			moveDisp.Z = circleCenter.Z - currentPos.Z;
		}

	if (isBullet)
	{
		s.StoredDisp = { currentPos.X + moveDisp.X, currentPos.Y + moveDisp.Y, currentPos.Z + moveDisp.Z };
	}
	else
	{
		pObject->SetLocation({ currentPos.X + moveDisp.X, currentPos.Y + moveDisp.Y, currentPos.Z + moveDisp.Z });
		if (pType->Vector_Freeze)
		{
			if (auto const pFoot = abstract_cast<FootClass*>(pObject))
				pFoot->StopMoving();
		}
	}

	if (pType->Vector_CircleEndOnMaxRadius && pType->Vector_CircleMaxRadius > 0 && s.CurrentCircleRadius >= pType->Vector_CircleMaxRadius)
		return;
	if (pType->Vector_CircleEndOnMinRadius && pType->Vector_CircleMinRadius > 0 && s.CurrentCircleRadius <= pType->Vector_CircleMinRadius)
		return;

	return;
}

// ================================================================
// MoveTo mode
// ================================================================
	if (static_cast<const CoordStruct&>(pType->Vector_MoveTo) != CoordStruct::Empty)
	{
		DirStruct moveDir = mainFacingDir;
		if (pType->Vector_AnglePerStep != 0.0)
		{
			if (s.MovementFrames == 1)
				s.CurrentAngle = 0.0;
			s.CurrentAngle += pType->Vector_AnglePerStep;
			moveDir = V_Radians2Dir(mainFacingDir.GetRadian<32>() + V_Deg2Rad(s.CurrentAngle));
		}

		CoordStruct grow{ static_cast<int>(pType->Vector_GrowRate.isset() ? pType->Vector_GrowRate.Get().X * s.MovementFrames : 0),
			static_cast<int>(pType->Vector_GrowRate.isset() ? pType->Vector_GrowRate.Get().Y * s.MovementFrames : 0),
			static_cast<int>(pType->Vector_GrowRate.isset() ? pType->Vector_GrowRate.Get().Z * s.MovementFrames : 0) };
		CoordStruct mt = pType->Vector_MoveTo;
	CoordStruct moveFlh = { mt.X + grow.X, mt.Y + grow.Y, mt.Z + grow.Z };

		CoordStruct moveDisp = V_FLHAbsoluteOffset(moveFlh, moveDir);

		if (isBullet)
		{
			s.StoredDisp = { currentPos.X + moveDisp.X, currentPos.Y + moveDisp.Y, currentPos.Z + moveDisp.Z };
		}
		else
		{
			pObject->SetLocation({ currentPos.X + moveDisp.X, currentPos.Y + moveDisp.Y, currentPos.Z + moveDisp.Z });
			if (pType->Vector_Freeze)
			{
				if (auto const pFoot = abstract_cast<FootClass*>(pObject))
					pFoot->StopMoving();
			}
		}
		return;
	}

	// ================================================================
	// TargetFLH modes: ReachTarget / Speed
	// ================================================================
	if (!pType->Vector_TargetFLH.isset())
	{
		if (!isBullet && pType->Vector_Freeze)
		{
			if (auto const pFoot = abstract_cast<FootClass*>(pObject))
				pFoot->StopMoving();
		}
		return;
	}

	CoordStruct frameTargetFlh = { pType->Vector_TargetFLH.Get().X + s.TargetOffset.X,
		pType->Vector_TargetFLH.Get().Y + s.TargetOffset.Y,
		pType->Vector_TargetFLH.Get().Z + s.TargetOffset.Z };
	//VecLog("[VEC] Frame=%d FLH=(%d,%d,%d) TGTFLH=(%d,%d,%d) OFFSET=(%d,%d,%d) ORIGIN=%d ONWORLD=%d ONBODY=%d\n",
	//	Unsorted::CurrentFrame,
	//	pType->Vector_TargetFLH.Get().X, pType->Vector_TargetFLH.Get().Y, pType->Vector_TargetFLH.Get().Z,
	//	frameTargetFlh.X, frameTargetFlh.Y, frameTargetFlh.Z,
	//	s.TargetOffset.X, s.TargetOffset.Y, s.TargetOffset.Z,
	//	static_cast<int>(pType->Vector_Origin.Get()),
	//	static_cast<int>(pType->Vector_OriginIsOnWorld.Get()),
	//	static_cast<int>(pType->Vector_OriginIsOnBody.Get()));
	CoordStruct frameTarget;

	switch (pType->Vector_Origin)
	{
	case VectorOrigin::Launcher:
		if (pInvoker)
			frameTarget = TechnoExt::GetFLHAbsoluteCoords(static_cast<TechnoClass*>(pInvoker), frameTargetFlh, !pType->Vector_OriginIsOnBody);
		else
			frameTarget = AttachEffectClass::GetFLHAbsoluteCoords(originPos, frameTargetFlh, mainFacingDir);
		break;
	case VectorOrigin::Self:
		if (pType->Vector_OriginIsOnWorld)
			frameTarget = AttachEffectClass::GetFLHAbsoluteCoords(originPos, frameTargetFlh, DirStruct{});
		else if (isBullet)
			frameTarget = AttachEffectClass::GetFLHAbsoluteCoords(currentPos, frameTargetFlh, mainFacingDir);
		else
			frameTarget = TechnoExt::GetFLHAbsoluteCoords(static_cast<TechnoClass*>(pObject), frameTargetFlh, !pType->Vector_OriginIsOnBody);
		break;
	default:
		frameTarget = AttachEffectClass::GetFLHAbsoluteCoords(originPos, frameTargetFlh, mainFacingDir);
		break;
	}

	CoordStruct dirVec = { frameTarget.X - currentPos.X, frameTarget.Y - currentPos.Y, frameTarget.Z - currentPos.Z };
	double dirLen = std::sqrt(static_cast<double>(dirVec.X * dirVec.X + dirVec.Y * dirVec.Y + dirVec.Z * dirVec.Z));
	CoordStruct resultDisp{ 0, 0, 0 };

	//VecLog("[VEC] CURR=(%d,%d,%d) FDIR=%.4f FACING=(%.4f,%.4f) FRMTGT=(%d,%d,%d) DIRVEC=(%d,%d,%d) DIRLEN=%.1f MOVFRM=%d\n",
	//	currentPos.X, currentPos.Y, currentPos.Z,
	//	mainFacingDir.GetRadian<32>(),
	//	effectiveFacing, effectiveTilt,
	//	frameTarget.X, frameTarget.Y, frameTarget.Z,
	//	dirVec.X, dirVec.Y, dirVec.Z,
	//	dirLen,
	//	s.MovementFrames);

	if (pType->Vector_ReachTarget)
	{
		int totalDuration = pType->Duration / pType->Vector_TimeStep;
		if (totalDuration < 1) totalDuration = 1;
		int effectiveDuration = totalDuration - pType->Vector_DisabledFrames;
		if (effectiveDuration < 1) effectiveDuration = 1;
		int remainingFrames = effectiveDuration - s.MovementFrames + 1;

		if (pType->Vector_ReachTargetEarlyEnd > 0 && pType->Vector_ReachTargetEarlyEnd < effectiveDuration
			&& remainingFrames <= pType->Vector_ReachTargetEarlyEnd)
		{
			s.DisabledTimer = -1;
			s.MovementFrames = effectiveDuration + 1; // 退出 EarlyEnd 区间，下一步 remainingFrames <= 0
		}

		if (remainingFrames <= 0)
		{
			if (pType->Vector_Force)
			{
				resultDisp.X = frameTarget.X - currentPos.X;
				resultDisp.Y = frameTarget.Y - currentPos.Y;
				resultDisp.Z = frameTarget.Z - currentPos.Z;
			}
		}
		else if (dirLen > 1e-6)
		{
			double adjustedSpeed = dirLen / remainingFrames;
			resultDisp.X = static_cast<int>(dirVec.X / dirLen * adjustedSpeed);
			resultDisp.Y = static_cast<int>(dirVec.Y / dirLen * adjustedSpeed);
			resultDisp.Z = static_cast<int>(dirVec.Z / dirLen * adjustedSpeed);

			if (s.ArcHeight != 0)
			{
				double t = static_cast<double>(s.MovementFrames) / effectiveDuration;
				double peakPct = s.ArcPeakPercent > 0 ? s.ArcPeakPercent : 0.5;
				double arcOffset;
				if (t <= peakPct)
				{
					double u = t / peakPct;
					arcOffset = s.ArcHeight * u * (2.0 - u);
				}
				else
				{
					double u = (t - peakPct) / (1.0 - peakPct);
					arcOffset = s.ArcHeight * (1.0 - u * u);
				}
				double baseX = s.InitialLocation.X + (frameTarget.X - s.InitialLocation.X) * t;
				double baseY = s.InitialLocation.Y + (frameTarget.Y - s.InitialLocation.Y) * t;
				double baseZ = s.InitialLocation.Z + (frameTarget.Z - s.InitialLocation.Z) * t;

				if (s.ArcRotation == 0.0)
				{
					resultDisp.X = static_cast<int>(baseX - currentPos.X);
					resultDisp.Y = static_cast<int>(baseY - currentPos.Y);
					resultDisp.Z = static_cast<int>(baseZ + arcOffset - currentPos.Z);
				}
				else
				{
					double ax = frameTarget.X - s.InitialLocation.X;
					double ay = frameTarget.Y - s.InitialLocation.Y;
					double az = frameTarget.Z - s.InitialLocation.Z;
					double aLen = std::sqrt(ax * ax + ay * ay + az * az);
					if (aLen > 1e-6)
					{
						double dnx = ax / aLen, dny = ay / aLen, dnz = az / aLen;
						double upDotD = dnz;
						double px = -dnx * upDotD, py = -dny * upDotD, pz = 1.0 - dnz * upDotD;
						double pLen = std::sqrt(px * px + py * py + pz * pz);
						if (pLen < 1e-6) { px = 1.0 - dnx * dnx; py = -dny * dnx; pz = -dnz * dnx; pLen = std::sqrt(px * px + py * py + pz * pz); }
						double pnx = px / pLen, pny = py / pLen, pnz = pz / pLen;
						double arcRad = V_Deg2Rad(s.ArcRotation);
						double c = std::cos(arcRad), s_arc = std::sin(arcRad);
						double rx = pnx * c + (dny * pnz - dnz * pny) * s_arc;
						double ry = pny * c + (dnz * pnx - dnx * pnz) * s_arc;
						double rz = pnz * c + (dnx * pny - dny * pnx) * s_arc;
						resultDisp.X = static_cast<int>(baseX + rx * arcOffset - currentPos.X);
						resultDisp.Y = static_cast<int>(baseY + ry * arcOffset - currentPos.Y);
						resultDisp.Z = static_cast<int>(baseZ + rz * arcOffset - currentPos.Z);
					}
					else
					{
						resultDisp.X = static_cast<int>(baseX - currentPos.X);
						resultDisp.Y = static_cast<int>(baseY - currentPos.Y);
						resultDisp.Z = static_cast<int>(baseZ + arcOffset - currentPos.Z);
					}
				}
			}
		}
	}
	else if (dirLen > 1e-6)
	{
		double spd = s.CurrentSpeed;
		if (pType->Vector_Acceleration != 0) spd += pType->Vector_Acceleration * s.CurrentFrame;
		if (pType->Vector_MinSpeed >= 0 && spd < pType->Vector_MinSpeed) spd = pType->Vector_MinSpeed;
		if (pType->Vector_MaxSpeed >= 0 && spd > pType->Vector_MaxSpeed) spd = pType->Vector_MaxSpeed;
		resultDisp.X = static_cast<int>(dirVec.X / dirLen * spd);
		resultDisp.Y = static_cast<int>(dirVec.Y / dirLen * spd);
		resultDisp.Z = static_cast<int>(dirVec.Z / dirLen * spd);
	}

	if (isBullet)
	{
		auto const pB = static_cast<BulletClass*>(pObject);
		s.StoredDisp = { currentPos.X + resultDisp.X, currentPos.Y + resultDisp.Y, currentPos.Z + resultDisp.Z };
		//VecLog("[VEC] RESULT DISP=(%d,%d,%d) STORED=(%d,%d,%d) VEL=(%.1f,%.1f,%.1f)\n",
		//	resultDisp.X, resultDisp.Y, resultDisp.Z,
		//	s.StoredDisp.X, s.StoredDisp.Y, s.StoredDisp.Z,
		//	pB->Velocity.X, pB->Velocity.Y, pB->Velocity.Z);
	}
	else
	{
		pObject->SetLocation({ currentPos.X + resultDisp.X, currentPos.Y + resultDisp.Y, currentPos.Z + resultDisp.Z });
		if (pType->Vector_Freeze)
		{
			if (auto const pFoot = abstract_cast<FootClass*>(pObject))
				pFoot->StopMoving();
		}
	}

	if (!isBullet && pType->Vector_SyncFacing && (resultDisp.X != 0 || resultDisp.Y != 0))
	{
		double ang = std::atan2(static_cast<double>(resultDisp.Y), static_cast<double>(resultDisp.X));
		auto dir = DirStruct(static_cast<short>(ang * 32768.0 / 3.14159265358979323846));
		static_cast<TechnoClass*>(pObject)->PrimaryFacing.SetDesired(dir);
	}

	if (pType->Vector_AllowFallingDestroy && pType->Vector_FallingDestroyHeight > 0)
	{
		if (s.InitialLocation.Z - GetPos().Z > pType->Vector_FallingDestroyHeight)
			return;
	}
}
