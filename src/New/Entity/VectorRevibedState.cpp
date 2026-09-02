#include "VectorRevibedState.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>

#include <BulletClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>
#include <FootClass.h>
#include <AircraftClass.h>
#include <Kamikaze.h>
#include <SpawnManagerClass.h>
#include <RocketLocomotionClass.h>
#include <LocomotionClass.h>
#include <MapClass.h>

#include <Ext/Techno/Body.h>

// ============================================================================
// 本地实现命名空间：轨迹算法自 Kratos VectorEffectReVibed.cpp 逐字照搬，
// 仅做 PhobosAI 基建适配（随机源/坐标 helper 本地化/缓存删除）。
// ============================================================================
namespace VectorRevibedImpl
{
	// ========================================================================
	// 随机（引擎同步，防 desync；分布语义与 Kratos 本地 mt19937 一致）
	// ========================================================================
	inline int Rnd(int min, int max)
	{
		if (max < min) std::swap(min, max);
		return ScenarioClass::Instance->Random.RandomRanged(min, max);
	}
	inline double RndD()
	{
		return ScenarioClass::Instance->Random.RandomDouble();
	}

	// ========================================================================
	// 坐标系 helper（原样搬 Kratos FLH.h / Status.cpp）
	// ========================================================================
	constexpr double BINARY_ANGLE_MAGIC = -(360.0 / (65535 - 1)) * (Math::Pi / 180);

	inline bool IsEmpty(const CoordStruct& c) { return c == CoordStruct::Empty; }

	// 引擎弧度→DirStruct（BINARY_ANGLE_MAGIC 为负，与 Kratos 一致）
	inline DirStruct Radians2Dir(double radians)
	{
		short d = (short)(radians / BINARY_ANGLE_MAGIC);
		return DirStruct{ d };
	}

	// RA2 坐标系：source→target 连线方向（含 -90° 修正，官方 API 语义）
	inline DirStruct Point2Dir(CoordStruct source, CoordStruct target)
	{
		double radians = Math::atan2(source.Y - target.Y, target.X - source.X);
		radians -= Math::deg2rad(90);
		return Radians2Dir(radians);
	}

	// 弹体朝向（沿 Velocity 方向）
	inline CoordStruct ToCoordStruct(const Vector3D<double>& v)
	{
		return CoordStruct{ static_cast<int>(v.X), static_cast<int>(v.Y), static_cast<int>(v.Z) };
	}
	inline DirStruct Facing(BulletClass* pBullet, CoordStruct location)
	{
		CoordStruct source = location;
		if (IsEmpty(location))
			source = pBullet->GetCoords();
		CoordStruct forward = source + ToCoordStruct(pBullet->Velocity);
		return Point2Dir(source, forward);
	}

	// FLH 偏移：Matrix3D 旋转 + Y 镜像 + lround 取整（Kratos 原样）
	inline Vector3D<float> GetFLHOffset(Matrix3D& mtx, CoordStruct flh)
	{
		mtx.Translate(Vector3D<float>{(float)flh.X, (float)flh.Y, (float)flh.Z});
		Vector3D<float> res = mtx * Vector3D<float>::Empty;
		res.Y *= -1; // 结果沿 X 镜像，镜像回来（Kerbiter）
		return res;
	}

	inline CoordStruct GetFLHAbsoluteOffset(CoordStruct flh, DirStruct dir, CoordStruct turretOffset = CoordStruct::Empty)
	{
		CoordStruct res = CoordStruct::Empty;
		if (flh != CoordStruct::Empty)
		{
			Matrix3D mtx;
			mtx.MakeIdentity();
			mtx.Translate(Vector3D<float>{(float)turretOffset.X, (float)turretOffset.Y, (float)turretOffset.Z});
			mtx.RotateZ((float)dir.GetRadian<32>());
			Vector3D<float> offset = GetFLHOffset(mtx, flh);
			res.X = std::lround(offset.X);
			res.Y = std::lround(offset.Y);
			res.Z = std::lround(offset.Z);
		}
		return res;
	}

	inline CoordStruct GetFLHAbsoluteCoords(CoordStruct source, CoordStruct flh, DirStruct dir, CoordStruct turretOffset = CoordStruct::Empty)
	{
		CoordStruct res = source;
		if (flh != CoordStruct::Empty)
			res += GetFLHAbsoluteOffset(flh, dir, turretOffset);
		return res;
	}

	// ========================================================================
	// 死亡/隐形判定（原样搬 Kratos Status.cpp）
	// ========================================================================
	inline bool IsDead(ObjectClass* pObject)
	{
		// 弹体 Health 可能 < 0
		return !pObject || pObject->Health == 0 || !pObject->IsAlive || !pObject->GetType();
	}
	inline bool IsDeadOrInvisible(ObjectClass* pObject)
	{
		return IsDead(pObject) || pObject->InLimbo;
	}
	inline bool IsDeadOrInvisible(TechnoClass* pTechno)
	{
		return !pTechno || pTechno->Health <= 0 || !pTechno->IsAlive
			|| pTechno->IsCrashing || pTechno->IsSinking || !pTechno->GetType()
			|| pTechno->InLimbo;
	}

	// ========================================================================
	// 目标有效性辅助（SpawnMissile 场景专用）——照搬旧版（Kratos）
	// ========================================================================

	// 从引擎 Kamikaze 控制器读导弹的目标点：
	// KamikazeControl->Cell 存目标格子——Homing 开启时每帧更新（目标活着跟随、死亡冻结），
	// 不 Homing 时由引擎写入目标 Cell。这是引擎眼中导弹真正要飞向的位置
	static bool TryGetKamikazeTarget(TechnoClass* pTechno, CoordStruct& out)
	{
		AircraftClass* pAircraft = pTechno ? abstract_cast<AircraftClass*>(pTechno) : nullptr;
		if (!pAircraft)
			return false;
		auto& nodes = Kamikaze::Instance.Nodes;
		for (int i = 0; i < nodes.Count; i++)
		{
			Kamikaze::KamikazeControl* pControl = nodes[i];
			if (pControl && pControl->Item == pAircraft && pControl->Cell)
			{
				out = pControl->Cell->GetCoords();
				return true;
			}
		}
		return false;
	}

	// 从 spawn 发射者的管理器读目标（Kamikaze Cell 的源头）：
	// 导弹未全部发射（未进 Kamikaze 容器）期间容器为空，直接读源头补全这一阶段
	static bool TryGetSpawnManagerTarget(TechnoClass* pTechno, CoordStruct& out)
	{
		if (pTechno && pTechno->SpawnOwner && !IsDeadOrInvisible(pTechno->SpawnOwner))
		{
			SpawnManagerClass* pSM = pTechno->SpawnOwner->SpawnManager;
			if (pSM)
			{
				if (pSM->Target)
				{
					out = pSM->Target->GetCoords();
					return true;
				}
			}
		}
		return false;
	}

	// Target 多级读取链：弹体目标 → 弹体落点 → 单位目标 → Kamikaze/SpawnManager
	// （PhobosAI 版砍掉 TechnoStatus 缓存，直接读引擎链）
	static bool GetTargetPosFromChain(BulletClass* pBullet, TechnoClass* pTechno, CoordStruct& out)
	{
		if (pBullet && pBullet->Target)
		{
			out = pBullet->Target->GetCoords(); // 抛射体目标 Cell 是真实落点
			return true;
		}
		if (pBullet)
		{
			out = pBullet->TargetCoords;
			return true;
		}
		if (pTechno && pTechno->Target)
		{
			out = pTechno->Target->GetCoords();
			return true;
		}
		if (pTechno)
		{
			CoordStruct kamikazePos{};
			if (TryGetKamikazeTarget(pTechno, kamikazePos) || TryGetSpawnManagerTarget(pTechno, kamikazePos))
			{
				out = kamikazePos;
				return true;
			}
		}
		return false;
	}

	// ========================================================================
	// 公共数学函数（行为等价：逐字照搬 Kratos 公式）
	// ========================================================================

	// 弧高二次曲线（ReachTarget / Speed 共用），t∈[0,1] 返回弧高绝对值
	inline double CalcArcOffsetAt(int height, double peakPercent, double t)
	{
		if (height == 0) return 0.0;
		if (t <= peakPercent)
		{
			double u = t / peakPercent;
			return height * u * (2.0 - u);
		}
		else
		{
			double u = (peakPercent < 1.0) ? (t - peakPercent) / (1.0 - peakPercent) : 0.0;
			return height * (1.0 - u * u);
		}
	}

	// 弧面旋转：把 arcDelta 按 Rodrigues 正交基分解到 XYZ（ReachTarget/Speed 共用）
	struct ArcDelta3D { double x, y, z; };
	inline ArcDelta3D RotateArcDelta(const CoordStruct& D, double rotDeg, double arcDelta)
	{
		ArcDelta3D out{ 0.0, 0.0, 0.0 };
		if (rotDeg == 0.0)
		{
			out.z = arcDelta;
			return out;
		}
		double dx = static_cast<double>(D.X);
		double dy = static_cast<double>(D.Y);
		double dz = static_cast<double>(D.Z);
		double dLen = std::sqrt(dx * dx + dy * dy + dz * dz);
		if (dLen <= 1e-6)
		{
			out.z = arcDelta;
			return out;
		}
		double dnx = dx / dLen, dny = dy / dLen, dnz = dz / dLen;
		double upDotD = dnz;
		double px = -dnx * upDotD, py = -dny * upDotD, pz = 1.0 - dnz * upDotD;
		double pLen = std::sqrt(px * px + py * py + pz * pz);
		if (pLen < 1e-6)
		{
			px = 1.0 - dnx * dnx; py = -dny * dnx; pz = -dnz * dnx;
			pLen = std::sqrt(px * px + py * py + pz * pz);
		}
		double pnx = px / pLen, pny = py / pLen, pnz = pz / pLen;
		double rad = Math::deg2rad(rotDeg);
		double c = std::cos(rad), s = std::sin(rad);
		out.x = (pnx * c + (dny * pnz - dnz * pny) * s) * arcDelta;
		out.y = (pny * c + (dnz * pnx - dnx * pnz) * s) * arcDelta;
		out.z = (pnz * c + (dnx * pny - dny * pnx) * s) * arcDelta;
		return out;
	}

	// 3D 法向量增量旋转（绕世界 F=Y / L=X / H=Z 轴，正速度=顺时针；主/大圆共用）
	inline void RotateNormal3D(double& nx, double& ny, double& nz,
		double stepF, double stepL, double stepH)
	{
		if (stepF != 0.0)
		{
			double rad = Math::deg2rad(stepF), c = std::cos(rad), s = std::sin(rad);
			double x = nx, z = nz;
			nx = x * c - z * s;
			nz = x * s + z * c;
		}
		if (stepL != 0.0)
		{
			double rad = Math::deg2rad(stepL), c = std::cos(rad), s = std::sin(rad);
			double y = ny, z = nz;
			ny = y * c + z * s;
			nz = -y * s + z * c;
		}
		if (stepH != 0.0)
		{
			double rad = Math::deg2rad(stepH), c = std::cos(rad), s = std::sin(rad);
			double x = nx, y = ny;
			nx = x * c + y * s;
			ny = -x * s + y * c;
		}
	}

	// 法线角速度解析：常数优先 → 区间2 50% 随机 → 区间1 随机 → 0（主/大圆共用）
	inline double ResolveAngleStep(double perStep, double m1, double M1, double m2, double M2)
	{
		if (perStep != 0.0) return perStep;
		if (M1 <= m1 && M2 <= m2) return 0.0;
		if (M2 > m2 && Rnd(0, 1))
			return m2 + (M2 - m2) * RndD();
		return M1 > m1 ? m1 + (M1 - m1) * RndD() : 0.0;
	}

	// 三态跟踪：NoUpdate=yes → 冻结 last；no + 单位存活 → 每帧快照 last；死亡 → 冻结 last
	inline CoordStruct TrackOriginCoord(ObjectClass* pUnit, bool noUpdate, CoordStruct& last)
	{
		if (!noUpdate && pUnit && !IsDeadOrInvisible(pUnit))
			last = pUnit->GetCoords();
		return last;
	}

	// ========================================================================
	// OnStart 子步骤（Kratos 原样，PhobosAI 参数化适配）
	// ========================================================================

	// 计时/快照/Duration/AcquireZ
	static void ParseCommon(VectorRevibedState& s, const VectorRevibedData& d, ObjectClass* pObject, int duration)
	{
		s._elapsedFrames = 0;
		s._moveFrame = 0;
		s._movementFrames = 0;
		s._effectiveTimeStep = d.TimeStep.Get();
		// _prevCircleCenter 不在此初始化：圆心追踪依赖 Origin 移动系统首帧的 skipOriginUpdate 赋值

		s._initialLocation = pObject->GetCoords();
		s._vectorAcquireZ = s._initialLocation.Z;  // Circle 圆心高度基准：获取 Vector 时的 Z
		s._totalDuration = duration / s._effectiveTimeStep;
	}

	// TargetOffset 随机偏移（Radius / F/L/H 两套 + Angles）——照搬旧版
	static void ParseTargetOffset(VectorRevibedState& s, const VectorRevibedData& d,
		ObjectClass* pObject, BulletClass* pBullet, TechnoClass* pTechno)
	{
		if (d.TargetOffsetRadiusMin < d.TargetOffsetRadiusMax)
		{
			// 半径模式：全向随机落点（与 F/L/H 互斥）
			double radius = (d.TargetOffsetRadiusMin2 < d.TargetOffsetRadiusMax2 && Rnd(0, 1))
				? Rnd(d.TargetOffsetRadiusMin2, d.TargetOffsetRadiusMax2)
				: Rnd(d.TargetOffsetRadiusMin, d.TargetOffsetRadiusMax);
			if (d.TargetOffsetSphere.Get())
			{
				// 球面均匀分布：z=2u-1 面积均匀 + 经度 2πv，避免极区聚集
				double u = RndD() * 2.0 - 1.0;
				double phi = RndD() * 2.0 * M_PI;
				double rXY = radius * std::sqrt(1.0 - u * u);
				s._randomTargetOffset.X = static_cast<int>(rXY * std::cos(phi));
				s._randomTargetOffset.Y = static_cast<int>(rXY * std::sin(phi));
				s._randomTargetOffset.Z = static_cast<int>(radius * u);
			}
			else
			{
				// XY 圆环：角度默认全向，TargetOffsetAngles 限制时按区间加权均匀
				double flhAngle;
				bool hasAngleRange = (d.TargetOffsetAngleMin < d.TargetOffsetAngleMax)
					|| (d.TargetOffsetAngleMin2 < d.TargetOffsetAngleMax2);
				if (hasAngleRange)
				{
					double len1 = d.TargetOffsetAngleMax - d.TargetOffsetAngleMin;
					double len2 = d.TargetOffsetAngleMax2 - d.TargetOffsetAngleMin2;
					double total = (len1 > 0 ? len1 : 0.0) + (len2 > 0 ? len2 : 0.0);
					double u = RndD() * total;
					double deg;
					if (len1 > 0 && u < len1)
						deg = d.TargetOffsetAngleMin + u;
					else
						deg = d.TargetOffsetAngleMin2 + (u - (len1 > 0 ? len1 : 0.0));
					bool hasBase = false;
					CoordStruct bulletPos = pObject->GetCoords();
					CoordStruct targetPos{};
					if (pBullet)
					{
						targetPos = pBullet->TargetCoords;
						hasBase = true;
					}
					else if (pTechno && pTechno->Target)
					{
						targetPos = pTechno->Target->GetCoords();
						hasBase = true;
					}
					if (hasBase)
					{
						double beta = std::atan2(bulletPos.Y - targetPos.Y, bulletPos.X - targetPos.X); // 近交点世界角
						double alpha = Point2Dir(targetPos, bulletPos).GetRadian<32>(); // 近交点 DirStruct 角
						// 消费端 mainFacingDir 统一取 DirStruct 原值（NoUpdate 不影响坐标系），直接复刻
						double mainFacingDirSim = alpha;
						flhAngle = -mainFacingDirSim - beta - Math::deg2rad(deg);
					}
					else
					{
						flhAngle = RndD() * 2.0 * M_PI; // 拿不到连线方向：回退全向
					}
				}
				else
				{
					flhAngle = RndD() * 2.0 * M_PI;  // 无角度限制：全向
				}
				if (!IsEmpty(d.TargetOffsetNormal.Get()))
				{
					// TargetOffsetNormal：随机落点在倾斜圆面上（法向量定义圆面），FLH 局部计算
					double fwX = static_cast<double>(d.TargetOffsetNormal.Get().Y); // L → X
					double fwY = static_cast<double>(d.TargetOffsetNormal.Get().X); // F → Y
					double fwZ = static_cast<double>(d.TargetOffsetNormal.Get().Z); // H → Z
					double lenXY = std::sqrt(fwX * fwX + fwY * fwY);
					double facing = lenXY > 1e-6 ? std::atan2(fwY, fwX) : 0.0;
					double tilt = lenXY > 1e-6 ? std::atan2(fwZ, lenXY) : (fwZ > 0 ? M_PI / 2.0 : -M_PI / 2.0);
					double cosF = std::cos(facing), sinF = std::sin(facing);
					double cosT = std::cos(tilt), sinT = std::sin(tilt);
					double rL = radius * std::cos(flhAngle);
					double rH = radius * std::sin(flhAngle);
					s._randomTargetOffset.X = static_cast<int>(rL * (-sinF) + rH * (-cosF * sinT));
					s._randomTargetOffset.Y = static_cast<int>(rL * cosF + rH * (-sinF * sinT));
					s._randomTargetOffset.Z = static_cast<int>(rH * cosT);
					// 选 B：倾斜面 Z 再叠加 TargetOffsetH 偏移（倾斜面 + 高度抖动）
					if (d.TargetOffsetHMin2 < d.TargetOffsetHMax2 && Rnd(0, 1))
						s._randomTargetOffset.Z += Rnd(d.TargetOffsetHMin2, d.TargetOffsetHMax2);
					else if (d.TargetOffsetHMin < d.TargetOffsetHMax)
						s._randomTargetOffset.Z += Rnd(d.TargetOffsetHMin, d.TargetOffsetHMax);
				}
				else
				{
					// 原水平圆环：X/Y 在 FL 平面，Z 独立用 TargetOffsetH 随机
					s._randomTargetOffset.X = static_cast<int>(radius * std::cos(flhAngle));
					s._randomTargetOffset.Y = static_cast<int>(radius * std::sin(flhAngle));
					s._randomTargetOffset.Z = (d.TargetOffsetHMin2 < d.TargetOffsetHMax2 && Rnd(0, 1))
						? Rnd(d.TargetOffsetHMin2, d.TargetOffsetHMax2)
						: (d.TargetOffsetHMin < d.TargetOffsetHMax
							? Rnd(d.TargetOffsetHMin, d.TargetOffsetHMax) : 0);
				}
			}
		}
		else
		{
			// F/L/H 模式：区间2有效且50%取区间2，否则区间1（无效给0）
			s._randomTargetOffset.X = (d.TargetOffsetFMin2 < d.TargetOffsetFMax2 && Rnd(0, 1))
				? Rnd(d.TargetOffsetFMin2, d.TargetOffsetFMax2)
				: (d.TargetOffsetFMin < d.TargetOffsetFMax
					? Rnd(d.TargetOffsetFMin, d.TargetOffsetFMax) : 0);
			s._randomTargetOffset.Y = (d.TargetOffsetLMin2 < d.TargetOffsetLMax2 && Rnd(0, 1))
				? Rnd(d.TargetOffsetLMin2, d.TargetOffsetLMax2)
				: (d.TargetOffsetLMin < d.TargetOffsetLMax
					? Rnd(d.TargetOffsetLMin, d.TargetOffsetLMax) : 0);
			s._randomTargetOffset.Z = (d.TargetOffsetHMin2 < d.TargetOffsetHMax2 && Rnd(0, 1))
				? Rnd(d.TargetOffsetHMin2, d.TargetOffsetHMax2)
				: (d.TargetOffsetHMin < d.TargetOffsetHMax
					? Rnd(d.TargetOffsetHMin, d.TargetOffsetHMax) : 0);
		}
	}

	// 弧参数三件套（rotation/height/peakPercent 随机解析）——照搬旧版
	// origin=false 主，true 大圆
	static void ParseArcParams(VectorRevibedState& s, const VectorRevibedData& d, bool origin)
	{
		if (!origin)
		{
			// --- 主弧参数 ---
			s._motion.arcRotation = d.ArcRotation.Get();
			if (d.ArcRandomRotationMax > d.ArcRandomRotationMin)
				s._motion.arcRotation = d.ArcRandomRotationMin + (d.ArcRandomRotationMax - d.ArcRandomRotationMin) * RndD();

			s._motion.arcHeight = d.ArcHeight.Get();
			if (d.ArcRandomHeightMax > d.ArcRandomHeightMin)
				s._motion.arcHeight = Rnd(d.ArcRandomHeightMin, d.ArcRandomHeightMax);

			s._motion.arcPeakPercent = d.ArcPeakPercent.Get() / 100.0;
			if (d.ArcPeakRandomPercent.Get().X < d.ArcPeakRandomPercent.Get().Y)
				s._motion.arcPeakPercent = Rnd(d.ArcPeakRandomPercent.Get().X, d.ArcPeakRandomPercent.Get().Y) / 100.0;
			if (s._motion.arcPeakPercent <= 0.0) s._motion.arcPeakPercent = 0.5;
			if (s._motion.arcPeakPercent >= 1.0) s._motion.arcPeakPercent = 0.5;
		}
		else
		{
			// --- 大圆弧参数（镜像主弧）---
			s._originMotion.arcRotation = d.OriginArcRotation.Get();
			if (d.OriginArcRandomRotationMax > d.OriginArcRandomRotationMin)
				s._originMotion.arcRotation = d.OriginArcRandomRotationMin + (d.OriginArcRandomRotationMax - d.OriginArcRandomRotationMin) * RndD();

			s._originMotion.arcHeight = d.OriginArcHeight.Get();
			if (d.OriginArcRandomHeightMax > d.OriginArcRandomHeightMin)
				s._originMotion.arcHeight = Rnd(d.OriginArcRandomHeightMin, d.OriginArcRandomHeightMax);

			s._originMotion.arcPeakPercent = d.OriginArcPeakPercent.Get() / 100.0;
			if (d.OriginArcPeakRandomPercent.Get().X < d.OriginArcPeakRandomPercent.Get().Y)
				s._originMotion.arcPeakPercent = Rnd(d.OriginArcPeakRandomPercent.Get().X, d.OriginArcPeakRandomPercent.Get().Y) / 100.0;
			if (s._originMotion.arcPeakPercent <= 0.0) s._originMotion.arcPeakPercent = 0.5;
			if (s._originMotion.arcPeakPercent >= 1.0) s._originMotion.arcPeakPercent = 0.5;
		}
	}

	// 单位是否 Jumpjet（Kratos GetLocoType==Jumpjet 等价，GUID memcmp 避免跨库 operator== 差异）
	static bool IsJumpjet(TechnoClass* pTechno)
	{
		if (!pTechno || !pTechno->GetTechnoType())
			return false;
		const _GUID& locoId = pTechno->GetTechnoType()->Locomotor;
		return std::memcmp(&locoId, &LocomotionClass::CLSIDs::Jumpjet, sizeof(_GUID)) == 0;
	}

	// 初始速度（LinearSpeed/单位 Speed/弹体 Speed/随机）——照搬旧版
	static void ParseSpeed(VectorRevibedState& s, const VectorRevibedData& d,
		ObjectClass* pObject, BulletClass* pBullet, TechnoClass* pTechno)
	{
		s._motion.speed = 0.0;
		if (d.LinearSpeed.Get() >= 0)
		{
			s._motion.speed = static_cast<double>(d.LinearSpeed.Get());
		}
		else if (pTechno)
		{
			TechnoTypeClass* pType = pTechno->GetTechnoType();
			if (IsJumpjet(pTechno))
				s._motion.speed = pType->JumpjetSpeed;
			else
				s._motion.speed = pType->Speed;
		}
		else if (pBullet)
		{
			s._motion.speed = pBullet->Speed;
		}
		// Speed 模式随机速度
		if (d.RandomSpeedMax > d.RandomSpeedMin)
		{
			s._motion.speed = Rnd(d.RandomSpeedMin, d.RandomSpeedMax);
		}
	}

	// 基础参考系 + 按 Origin 锁定基线——照搬旧版（砍 TechnoStatus 缓存）
	static void InitOrigin(VectorRevibedState& s, const VectorRevibedData& d,
		ObjectClass* pObject, BulletClass* pBullet, TechnoClass* pTechno,
		ObjectClass* pLauncher, ObjectClass* pSource)
	{
		// --- 基础参考系（始终赋值，供 OriginOrigin 等使用） ---
		if (pBullet)
		{
			s._pLauncher = pBullet->Owner;
			if (pSource) s._pSource = pSource;
		}
		else if (pTechno)
		{
			s._pLauncher = pSource ? pSource : pTechno; // 单位侧用攻击者作为Launcher
		}

		// --- Origin 初始化：锁定基线 ---
		switch (d.Origin.Get())
		{
		case VectorOrigin::Target:
			// 无论 NoUpdate 都锁定基线（挂载时引擎目标还是真实的）：NoUpdate=yes 直接用它，
			// no 每帧刷新覆盖；引擎目标失效时回退此基线（保证打地面轨迹一致）
			// 锚点 = 引擎 Kamikaze 控制器存的目标点 → 攻击目标 → 自身
			if (pTechno)
			{
				CoordStruct kamikazePos{};
				bool gotKamikaze = TryGetKamikazeTarget(pTechno, kamikazePos);
				if (gotKamikaze)
				{
					s._initialOriginPos = kamikazePos;
				}
				else if (TryGetSpawnManagerTarget(pTechno, s._initialOriginPos))
				{
					// 未进 Kamikaze 容器（导弹未全部发射）时读源头：SpawnManager 目标
				}
				else if (pTechno->Target)
				{
					s._initialOriginPos = pTechno->Target->GetCoords();
				}
				else
				{
					// Kamikaze 容器此刻可能还没加入导弹（发射后才加入）：不锁自身，
					// 留空由 Step 首帧补读
					s._initialOriginPos = CoordStruct::Empty;
				}
			}
			else if (pBullet)
			{
				s._initialOriginPos = pBullet->TargetCoords;
			}
			else
			{
				s._initialOriginPos = pObject->GetCoords();
			}
			break;

		case VectorOrigin::Launcher:
			// 无论 NoUpdate 都锁定基线（与 Target 分支一致）：NoUpdate=yes 直接用，
			// no 每帧快照刷新覆盖；launcher 死亡时冻结此基线作为 origin 解算起点
			if (pBullet && pBullet->Owner)
				s._initialOriginPos = pBullet->Owner->GetCoords();
			else if (pTechno)
				s._initialOriginPos = pTechno->GetCoords();
			else
				s._initialOriginPos = pObject->GetCoords();
			break;

		case VectorOrigin::Source:
			// 无论 NoUpdate 都锁定基线（与 Target/Launcher 分支一致）：死亡时冻结此基线
			if (pSource)
				s._initialOriginPos = pSource->GetCoords();
			else
				s._initialOriginPos = pObject->GetCoords(); // 兜底与 Target 分支一致
			if (pSource)
				s._pSource = pSource;
			break;

		case VectorOrigin::Self:
			if (pBullet)
			{
				double bulletRad = std::atan2(pBullet->Velocity.X, pBullet->Velocity.Y);
				DirStruct bulletFacing;
				bulletFacing.SetValue<16>(static_cast<size_t>(bulletRad * 32768.0 / M_PI));
				s._initialOriginPos = GetFLHAbsoluteCoords(pBullet->GetCoords(), d.OriginFLH.Get(), bulletFacing);
				s._facingDir = bulletFacing; // 锁定初始朝向
				s._facingRad = s._facingDir.GetRadian<32>();
			}
			else if (pTechno)
			{
				CoordStruct unitPos = pTechno->GetCoords();
				DirStruct unitFacing = pTechno->TurretFacing();
				s._initialOriginPos = GetFLHAbsoluteCoords(unitPos, d.OriginFLH.Get(), unitFacing);
				s._facingDir = unitFacing; // 锁定初始朝向
				s._facingRad = s._facingDir.GetRadian<32>();
			}
			break;
		}
	}

	// 朝向锁定：NormalVector 处理 + 法向量初始化 + 角速度解析 + 非 Normal 的 Origin 朝向——照搬旧版
	static void LockFacing(VectorRevibedState& s, const VectorRevibedData& d,
		ObjectClass* pObject, BulletClass* pBullet, TechnoClass* pTechno,
		ObjectClass* pLauncher, ObjectClass* pSource)
	{
		// NormalVector 设为后，F 轴完全由向量决定，Origin 只控制原点位置
		bool hasNormal = !IsEmpty(d.NormalVector.Get())
			|| d.NormalRandomF.Get().Y > d.NormalRandomF.Get().X
			|| d.NormalRandomL.Get().Y > d.NormalRandomL.Get().X
			|| d.NormalRandomH.Get().Y > d.NormalRandomH.Get().X;

		// --- 锁定 FLH 旋转朝向（挂载时固定） ---
		// NormalVector 使用 FLH 坐标系：F=南北(X→世界Y)，L=东西(Y→世界X)，H=Z
		if (hasNormal)
		{
			double fwY = static_cast<double>(d.NormalVector.Get().X);  // F → 世界 Y（北）
			double fwX = static_cast<double>(d.NormalVector.Get().Y);  // L → 世界 X（东）
			double fwZ = static_cast<double>(d.NormalVector.Get().Z);  // H → Z

			// 随机分量
			if (d.NormalRandomF.Get().Y > d.NormalRandomF.Get().X)
				fwY = Rnd(d.NormalRandomF.Get().X, d.NormalRandomF.Get().Y);
			if (d.NormalRandomL.Get().Y > d.NormalRandomL.Get().X)
				fwX = Rnd(d.NormalRandomL.Get().X, d.NormalRandomL.Get().Y);
			if (d.NormalRandomH.Get().Y > d.NormalRandomH.Get().X)
				fwZ = Rnd(d.NormalRandomH.Get().X, d.NormalRandomH.Get().Y);

			double lenXY = std::sqrt(fwX * fwX + fwY * fwY);
			s._facingRad = lenXY > 1e-6 ? std::atan2(fwY, fwX) : 0.0;
			s._tiltRad = lenXY > 1e-6 ? std::atan2(fwZ, lenXY) : (fwZ > 0 ? M_PI / 2.0 : -M_PI / 2.0);
		}
		else
		{
			s._tiltRad = 0.0;
		}

		// 法线旋转角速度解析（常数优先，否则随机）
		s._motion.normalStepF = ResolveAngleStep(d.NormalFAnglePerStep.Get(), d.NormalFAngleRMin, d.NormalFAngleRMax, d.NormalFAngleRMin2, d.NormalFAngleRMax2);
		s._motion.normalStepL = ResolveAngleStep(d.NormalLAnglePerStep.Get(), d.NormalLAngleRMin, d.NormalLAngleRMax, d.NormalLAngleRMin2, d.NormalLAngleRMax2);
		s._motion.normalStepH = ResolveAngleStep(d.NormalHAnglePerStep.Get(), d.NormalHAngleRMin, d.NormalHAngleRMax, d.NormalHAngleRMin2, d.NormalHAngleRMax2);
		s._motion.lissajousStep = d.Lissajous.Get();

		// 初始化 3D 法向量（从球坐标 _facingRad/_tiltRad 转换）
		{
			double ct = std::cos(s._tiltRad), st = std::sin(s._tiltRad);
			double cf = std::cos(s._facingRad), sf = std::sin(s._facingRad);
			s._motion.normalX = ct * cf;
			s._motion.normalY = ct * sf;
			s._motion.normalZ = st;
		}

		// --- 非 Normal 的 Origin 朝向锁定 ---
		if (!hasNormal)
		{
			switch (d.Origin.Get())
			{
			case VectorOrigin::Launcher:
			{
				TechnoClass* pLauncherTechno = abstract_cast<TechnoClass*>(s._pLauncher);
				if (pLauncherTechno && !IsDeadOrInvisible(pLauncherTechno))
				{
					if (d.IsOnOrigin)
					{
						s._facingDir = d.OriginIsOnBody.Get()
							? pLauncherTechno->PrimaryFacing.Current()
							: pLauncherTechno->TurretFacing();
					}
					else
					{
						s._facingDir = Point2Dir(pLauncherTechno->GetCoords(), pObject->GetCoords()); // 发射者→弹体连线
					}
					s._facingRad = s._facingDir.GetRadian<32>();
				}
				break;
			}

			case VectorOrigin::Target:
			{
				// F 轴：yes=目标单位自身朝向（目标无朝向/格子时回退连线），no=目标→弹体连线
				CoordStruct targetPos{};
				bool hasTarget = false;
				if (pTechno && pTechno->Target)
				{
					targetPos = pTechno->Target->GetCoords();
					hasTarget = true;
				}
				else if (pBullet)
				{
					targetPos = pBullet->TargetCoords;
					hasTarget = true;
				}
				if (hasTarget)
				{
					if (d.IsOnOrigin)
					{
						AbstractClass* pTgt = pTechno ? pTechno->Target : (pBullet ? pBullet->Target : nullptr);
						TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTgt);
						if (pTargetTechno && !IsDeadOrInvisible(pTargetTechno))
						{
							s._facingDir = d.OriginIsOnBody.Get()
								? pTargetTechno->PrimaryFacing.Current()
								: pTargetTechno->TurretFacing();
							s._facingRad = s._facingDir.GetRadian<32>();
							break;
						}
						// 目标无朝向（格子）：回退连线
					}
					s._facingDir = Point2Dir(targetPos, pObject->GetCoords()); // 目标→弹体连线
					s._facingRad = s._facingDir.GetRadian<32>();
				}
				break;
			}

			case VectorOrigin::Source:
			{
				// F 轴：yes=Source 单位自身朝向（无朝向回退连线），no=Source→弹体连线
				if (pSource)
				{
					CoordStruct sourcePos = pSource->GetCoords();
					if (d.IsOnOrigin)
					{
						TechnoClass* pSourceTechno = abstract_cast<TechnoClass*>(pSource);
						if (pSourceTechno && !IsDeadOrInvisible(pSourceTechno))
						{
							s._facingDir = d.OriginIsOnBody.Get()
								? pSourceTechno->PrimaryFacing.Current()
								: pSourceTechno->TurretFacing();
							s._facingRad = s._facingDir.GetRadian<32>();
							break;
						}
						// 来源无朝向：回退连线
					}
					s._facingDir = Point2Dir(sourcePos, pObject->GetCoords()); // Source→弹体连线
					s._facingRad = s._facingDir.GetRadian<32>();
				}
				break;
			}

			default: // FLH：抛射体自身朝向（Self 无"另一单位朝向"，IsOnOrigin 不区分）
			{
				if (pBullet)
					s._facingRad = std::atan2(pBullet->Velocity.X, pBullet->Velocity.Y);
				else if (pTechno)
					s._facingRad = pTechno->TurretFacing().GetRadian<32>();
				break;
			}
			}
		}

		// TargetOffsetNormal 世界固定（IsNormalOnOrigin=no）：把 FLH 落点按锁定朝向转成世界坐标，
		// 消费端把偏移叠加在旋转后的 TargetFLH 上，不随 F 轴（单位朝向）转动。
		if (!d.IsNormalOnOrigin.Get() && !IsEmpty(d.TargetOffsetNormal.Get()))
		{
			s._randomTargetOffset = GetFLHAbsoluteCoords(CoordStruct::Empty, s._randomTargetOffset, s._facingDir);
		}
	}

} // namespace VectorRevibedImpl

// ============================================================================
// Init（对应 Kratos OnStart 编排）
// ============================================================================
void VectorRevibedAI_Init(VectorRevibedState& s, const VectorRevibedData& d,
	ObjectClass* pObject, ObjectClass* pLauncher, ObjectClass* pSource, int duration)
{
	using namespace VectorRevibedImpl;

	BulletClass* pBullet = abstract_cast<BulletClass*>(pObject);
	TechnoClass* pTechno = abstract_cast<TechnoClass*>(pObject);
	if (pTechno && pTechno->WhatAmI() == AbstractType::Building)
		return; // 建筑不挂载（防御；PhobosAI 仅 Projectile 挂载，正常不会到这）

	ParseCommon(s, d, pObject, duration);

	// 影子坐标（Speed 模式弧高进度基准，不受弧高 Z 偏移污染）
	s._motion.shadowX = s._initialLocation.X;
	s._motion.shadowY = s._initialLocation.Y;
	s._motion.shadowZ = s._initialLocation.Z;
	s._motion.shadowTraveled = 0.0;

	ParseTargetOffset(s, d, pObject, pBullet, pTechno);
	ParseArcParams(s, d, false); // 主弧
	ParseArcParams(s, d, true);  // 大圆弧
	ParseSpeed(s, d, pObject, pBullet, pTechno);
	InitOrigin(s, d, pObject, pBullet, pTechno, pLauncher, pSource);
	LockFacing(s, d, pObject, pBullet, pTechno, pLauncher, pSource);
}

// ============================================================================
// Step（对应 Kratos GetVectorResult，段落化拆分）
// ============================================================================
namespace VectorRevibedImpl
{
	// ------------------------------------------------------------------------
	// 主运动：hasCircle 分支（Circle 模式 + Origin 圆心运动系统）
	// 返回 true 表示本帧已处理完毕（对应 Kratos 的 return result）
	// ------------------------------------------------------------------------
	static bool StepCircle(VectorRevibedState& s, const VectorRevibedData& d,
		BulletClass* pBullet, TechnoClass* pTechno, ObjectClass* pObject, ObjectClass* pSource,
		const CoordStruct& currentPos, const CoordStruct& originPos,
		double effectiveFacing, double effectiveTilt,
		double originTerrainTilt, bool hasNormal, int duration, VectorRevibedResult& out)
	{
		// 三选二：缺半径用当前XY距离，缺速度用半径×角速度，缺角速度用速度/半径
		double calcRadius = static_cast<double>(d.CircleRadius.Get());
		if (calcRadius <= 0.0)
		{
			double tdx = currentPos.X - originPos.X;
			double tdy = currentPos.Y - originPos.Y;
			calcRadius = std::sqrt(tdx * tdx + tdy * tdy);
		}
		if (calcRadius < 1.0)
			calcRadius = 1.0;  // 防止除零：半径 + 角速度互推时必 > 0

		// 动态线速：每帧叠加加速度（初始值已在 DisabledFrames 前预初始化）
		s._motion.circleSpeed += d.CircleSpeedAcceleration.Get();
		if (d.CircleMaxSpeed.Get() != 0 && s._motion.circleSpeed > d.CircleMaxSpeed.Get())
			s._motion.circleSpeed = static_cast<double>(d.CircleMaxSpeed.Get());
		if (d.CircleMinSpeed.Get() != 0 && s._motion.circleSpeed < d.CircleMinSpeed.Get())
			s._motion.circleSpeed = static_cast<double>(d.CircleMinSpeed.Get());

		// 角速度动态：每帧叠加加速度（初始值已在 DisabledFrames 前预初始化）
		s._motion.circleAngle += d.CircleAngleAcceleration.Get();
		if (d.CircleMaxAngle.Get() != 0.0 && s._motion.circleAngle > d.CircleMaxAngle.Get())
			s._motion.circleAngle = d.CircleMaxAngle.Get();
		if (d.CircleMinAngle.Get() != 0.0 && s._motion.circleAngle < d.CircleMinAngle.Get())
			s._motion.circleAngle = d.CircleMinAngle.Get();

		double speed = s._motion.circleSpeed;
		double angleStep = s._motion.circleAngle;

		// 三选二：半径 + 角速度优先，两者都有时速率由角速度推算（忽略显式 CircleSpeed）
		if (angleStep > 0.0)
			speed = calcRadius * Math::deg2rad(angleStep);
		else if (speed > 0.0)
			angleStep = Math::rad2deg(speed / calcRadius);

		// 圆心 = Origin + CircleOrigin 偏移（世界坐标系）
		// CircleOrigin Z 高度规则：CircleOrigin 非空 → 绝对覆写 OriginFLH.Z+CircleOrigin.Z；
		// 仅 OriginFLH 非空 → 相对偏移 _vectorAcquireZ+OriginFLH.Z
		CoordStruct adjustedCircleOrigin = d.CircleOrigin.Get();
		if (!IsEmpty(d.CircleOrigin.Get()))
			adjustedCircleOrigin.Z = d.OriginFLH.Get().Z + d.CircleOrigin.Get().Z;

		CoordStruct circleCenter = originPos;

		// AllowOriginTilt：用三维 FLH 旋转替代二维 GetFLHAbsoluteCoords
		if (d.AllowOriginTilt.Get() && !IsEmpty(d.OriginFLH.Get()) && d.Origin.Get() != VectorOrigin::Self)
		{
			int f = d.OriginFLH.Get().X, l = d.OriginFLH.Get().Y, h = d.OriginFLH.Get().Z;
			double sinT = std::sin(originTerrainTilt), cosT = std::cos(originTerrainTilt);
			// FLH 旋转用单位自身 facing（_facingRad 是法向量方向，不适用于 FLH 的 F/L 分量）
			TechnoClass* pOriginTechno = nullptr;
			switch (d.Origin.Get())
			{
			case VectorOrigin::Target:
				if (pBullet && pBullet->Target)
					pOriginTechno = abstract_cast<TechnoClass*>(pBullet->Target);
				else if (pTechno && pTechno->Target)
					pOriginTechno = abstract_cast<TechnoClass*>(pTechno->Target);
				break;
			case VectorOrigin::Source:
				pOriginTechno = abstract_cast<TechnoClass*>(s._pSource);
				break;
			case VectorOrigin::Launcher:
				pOriginTechno = abstract_cast<TechnoClass*>(s._pLauncher);
				break;
			case VectorOrigin::Self:
				pOriginTechno = pTechno;
				break;
			}
			double unitF = (pOriginTechno && !IsDeadOrInvisible(pOriginTechno))
				? pOriginTechno->PrimaryFacing.Current().GetRadian<32>() : 0.0;
			double cosF = std::cos(unitF), sinF = std::sin(unitF);
			// 先绕 L 轴（左右轴）转 tilt（俯仰），再绕 Z 轴转 facing
			double fTilt = f * cosT - h * sinT;
			double hTilt = f * sinT + h * cosT;
			circleCenter.X += static_cast<int>(fTilt * cosF - l * sinF);
			circleCenter.Y += static_cast<int>(fTilt * sinF + l * cosF);
			circleCenter.Z += static_cast<int>(hTilt);
		}

		if (!IsEmpty(d.CircleOrigin.Get()))
		{
			if (d.AllowOriginTilt.Get())
			{
				// FLH→世界坐标（facing + terrainTilt 三维旋转），替换 GetFLHAbsoluteCoords（仅二维）
				double cosF = std::cos(effectiveFacing), sinF = std::sin(effectiveFacing);
				double cosT = std::cos(originTerrainTilt), sinT = std::sin(originTerrainTilt);
				double f = static_cast<double>(adjustedCircleOrigin.X);
				double l = static_cast<double>(adjustedCircleOrigin.Y);
				double h = static_cast<double>(adjustedCircleOrigin.Z);
				double fTilt = f * cosT - h * sinT;
				double hTilt = f * sinT + h * cosT;
				circleCenter.X += static_cast<int>(fTilt * cosF - l * sinF);
				circleCenter.Y += static_cast<int>(fTilt * sinF + l * cosF);
				circleCenter.Z += static_cast<int>(hTilt);
			}
			else
			{
				circleCenter = originPos + adjustedCircleOrigin;
			}
		}
		else if (!IsEmpty(d.OriginFLH.Get()))
		{
			// 仅 OriginFLH：CircleOrigin 为空时不走 FLH 转换，手动设 Z
			circleCenter.Z = s._vectorAcquireZ + d.OriginFLH.Get().Z;
		}

		// 圆心移动：Vector.Origin.* 系统
		if (!IsEmpty(d.OriginMoveTo.Get()) || d.OriginReachTarget.Get() || d.OriginLinearSpeed.Get() >= 0
			|| !IsEmpty(d.OriginTargetFLH.Get())
			|| d.OriginCircleRadius.Get() >= 0 || d.OriginCircleSpeed.Get() != 0 || d.OriginCircleAnglePerStep.Get() != 0)
		{
			// 基座：默认 originPos，OriginOrigin 可替换为独立参考系
			CoordStruct baseCenter = originPos;
			if (d.OriginOrigin.Get() != VectorOrigin::Self)
			{
				switch (d.OriginOrigin.Get())
				{
				case VectorOrigin::Launcher:
					if (s._pLauncher && !IsDeadOrInvisible(s._pLauncher))
					{
						if (!d.OriginOriginNoUpdate.Get())
							s._initialBaseCenter = s._pLauncher->GetCoords(); // 每帧快照（NoUpdate=yes 冻结首帧不更新）
						baseCenter = s._pLauncher->GetCoords();
					}
					else
						baseCenter = s._initialBaseCenter; // 发射者死亡：冻结快照，不再读指针
					break;
				case VectorOrigin::Target:
					{
						CoordStruct targetBase{};
						bool gotTargetBase = false;
						if (pTechno && pTechno->Target)
						{
							targetBase = pTechno->Target->GetCoords();
							gotTargetBase = true;
						}
						else if (pTechno)
						{
							FootClass* pFoot = abstract_cast<FootClass*>(pTechno);
							if (pFoot && pFoot->Destination)
							{
								targetBase = pFoot->Destination->GetCoords();
								gotTargetBase = true;
							}
						}
						else if (pBullet && pBullet->Target)
						{
							targetBase = pBullet->Target->GetCoords();
							gotTargetBase = true;
						}
						else if (pBullet && pBullet->Owner && pBullet->Owner->Target)
						{
							targetBase = pBullet->Owner->Target->GetCoords();
							gotTargetBase = true;
						}
						else if (pBullet)
						{
							targetBase = pBullet->TargetCoords;
							gotTargetBase = true;
						}
						if (gotTargetBase)
						{
							if (!d.OriginOriginNoUpdate.Get())
								s._initialBaseCenter = targetBase; // 每帧快照（NoUpdate=yes 冻结首帧不更新）
							baseCenter = targetBase;
						}
						else
							baseCenter = s._initialBaseCenter; // 目标失效：冻结快照，不再掉回 originPos
					}
					break;
				case VectorOrigin::Source:
					if (s._pSource && !IsDeadOrInvisible(s._pSource))
					{
						if (!d.OriginOriginNoUpdate.Get())
							s._initialBaseCenter = s._pSource->GetCoords(); // 每帧快照（NoUpdate=yes 冻结首帧不更新）
						baseCenter = s._pSource->GetCoords();
					}
					else
						baseCenter = s._initialBaseCenter; // 来源死亡：冻结快照，不再读指针
					break;
				}
			}
			else if (!IsEmpty(d.OriginOriginFLH.Get()))
			{
				baseCenter.X += d.OriginOriginFLH.Get().X;
				baseCenter.Y += d.OriginOriginFLH.Get().Y;
				baseCenter.Z += d.OriginOriginFLH.Get().Z;
			}

			// Origin.CircleOffset 世界偏移
			if (!IsEmpty(d.OriginCircleOffset.Get()))
				baseCenter = baseCenter + d.OriginCircleOffset.Get();

			// OriginNoUpdate：首帧快照基座，后续帧冻结
			if (s._elapsedFrames == 0)
				s._initialBaseCenter = baseCenter;
			else if (d.OriginOriginNoUpdate.Get())
				baseCenter = s._initialBaseCenter;

			if (s._elapsedFrames == 0)
			{
				// 初始偏移 = 0：大圆圆心直接用大圆基座（baseCenter，已含 Origin.CircleOrigin 偏移），
				// 不绑定主圆圆心（circleCenter）
				s._originOffset = {};
				// Circle 初始化
				s._originMotion.circleRadius = d.OriginCircleRadius.Get();
				s._originMotion.circleSpeed = d.OriginCircleSpeed.Get();
				s._originMotion.angle = 0.0; // 初始相位
				// 未显式设半径：取当前偏移的水平距离
				if (s._originMotion.circleRadius < 0)
					s._originMotion.circleRadius = (int)std::sqrt(
						(double)s._originOffset.X * s._originOffset.X +
						(double)s._originOffset.Y * s._originOffset.Y +
						(double)s._originOffset.Z * s._originOffset.Z);
				// 随机
				if (d.OriginCircleRandomRadiusMax > d.OriginCircleRandomRadiusMin)
					s._originMotion.circleRadius = Rnd(d.OriginCircleRandomRadiusMin, d.OriginCircleRandomRadiusMax);
				if (d.OriginCircleRandomAngleMax > d.OriginCircleRandomAngleMin)
					s._originMotion.angle = d.OriginCircleRandomAngleMin + (d.OriginCircleRandomAngleMax - d.OriginCircleRandomAngleMin) * RndD();
				// Target 随机偏移
				s._originTargetOffset.X = Rnd(d.OriginTargetOffsetFMin, d.OriginTargetOffsetFMax);
				s._originTargetOffset.Y = Rnd(d.OriginTargetOffsetLMin, d.OriginTargetOffsetLMax);
				s._originTargetOffset.Z = Rnd(d.OriginTargetOffsetHMin, d.OriginTargetOffsetHMax);
				// Normal 初始化
				if (!IsEmpty(d.OriginNormalVector.Get()))
				{
					double fy = d.OriginNormalVector.Get().X, fx = d.OriginNormalVector.Get().Y, fz = d.OriginNormalVector.Get().Z;
					if (d.OriginNormalRandomF.Get().Y > d.OriginNormalRandomF.Get().X) fy = Rnd(d.OriginNormalRandomF.Get().X, d.OriginNormalRandomF.Get().Y);
					if (d.OriginNormalRandomL.Get().Y > d.OriginNormalRandomL.Get().X) fx = Rnd(d.OriginNormalRandomL.Get().X, d.OriginNormalRandomL.Get().Y);
					if (d.OriginNormalRandomH.Get().Y > d.OriginNormalRandomH.Get().X) fz = Rnd(d.OriginNormalRandomH.Get().X, d.OriginNormalRandomH.Get().Y);
					double len = std::sqrt(fx*fx+fy*fy);
					s._originFacing = len>1e-6 ? std::atan2(fy,fx) : 0;
					s._originTilt = len>1e-6 ? std::atan2(fz,len) : (fz>0?M_PI/2.0:-M_PI/2.0);
				}
				// Normal 角速度
				s._originMotion.normalStepF = ResolveAngleStep(d.OriginNormalFAnglePerStep.Get(), d.OriginNormalFAngleRMin, d.OriginNormalFAngleRMax, d.OriginNormalFAngleRMin2, d.OriginNormalFAngleRMax2);
				s._originMotion.normalStepL = ResolveAngleStep(d.OriginNormalLAnglePerStep.Get(), d.OriginNormalLAngleRMin, d.OriginNormalLAngleRMax, d.OriginNormalLAngleRMin2, d.OriginNormalLAngleRMax2);
				s._originMotion.normalStepH = ResolveAngleStep(d.OriginNormalHAnglePerStep.Get(), d.OriginNormalHAngleRMin, d.OriginNormalHAngleRMax, d.OriginNormalHAngleRMin2, d.OriginNormalHAngleRMax2);
				s._originMotion.lissajousStep = d.OriginLissajous.Get();
				// 无 OriginNormalVector 时：默认水平圆面（法向量朝上）
				if (IsEmpty(d.OriginNormalVector.Get()))
				{
					s._originFacing = 0;
					s._originTilt = M_PI / 2.0;
					// OriginAllowCircleTilt: 大圆面跟随目标倾斜（Origin=Target 时有效）
					if (d.OriginAllowCircleTilt.Get() && d.OriginOrigin.Get() == VectorOrigin::Target)
					{
						CoordStruct targetPos {};
						bool hasTargetPos = false;
						if (pBullet) { targetPos = pBullet->TargetCoords; hasTargetPos = true; }
						else if (pTechno && pTechno->Target) { targetPos = pTechno->Target->GetCoords(); hasTargetPos = true; }
						if (hasTargetPos)
						{
							double dx = circleCenter.X - targetPos.X;
							double dy = circleCenter.Y - targetPos.Y;
							double dz = circleCenter.Z - targetPos.Z;
							double lenXY = std::sqrt(dx * dx + dy * dy);
							s._originTilt = (lenXY > 1e-6) ? std::atan2(dz, lenXY) : M_PI / 2.0;
						}
					}
				}
				// 有 OriginNormalVector 时：facing/tilt 均取它的 F/L/H 分量（彻底世界固定）。
				// 锁定基础法向量球坐标（OriginIsNormalOnOrigin 每帧旋转的基准，不被 OriginAllowCircleTilt 覆盖污染）
				s._baseOriginFacing = s._originFacing;
				s._baseOriginTilt = s._originTilt;
				// 初始化大圆 3D 法向量
				{
					double ct = std::cos(s._originTilt), st = std::sin(s._originTilt);
					double cf = std::cos(s._originFacing), sf = std::sin(s._originFacing);
					s._originMotion.normalX = ct * cf;
					s._originMotion.normalY = ct * sf;
					s._originMotion.normalZ = st;
				}
			}
			// OriginIsNormalOnOrigin：大圆法向量随 OriginOrigin 单位转动（facing + tilt 全跟随，同主圆）。
			// 基础 = 首帧锁定的 _baseOriginFacing/_baseOriginTilt，每帧按单位朝向 + 倾斜转动（Rodrigues）。
			// 不回写成员 _originFacing/_originTilt（保持首帧锁定值，段外消费点从法向量现算球坐标）。
			if (d.OriginIsNormalOnOrigin.Get() && !IsEmpty(d.OriginNormalVector.Get()))
			{
				double facingU = s._originFacing;
				switch (d.OriginOrigin.Get())
				{
				case VectorOrigin::Launcher:
					{
						TechnoClass* pLT = abstract_cast<TechnoClass*>(s._pLauncher);
						if (pLT && !IsDeadOrInvisible(pLT))
							facingU = pLT->TurretFacing().GetRadian<32>();
						else if (pTechno) facingU = pTechno->TurretFacing().GetRadian<32>();
					}
					break;
				case VectorOrigin::Target:
					{
						AbstractClass* pTgt = pBullet ? pBullet->Target : (pTechno ? pTechno->Target : nullptr);
						TechnoClass* pTT = abstract_cast<TechnoClass*>(pTgt);
						if (pTT && !IsDeadOrInvisible(pTT))
							facingU = pTT->TurretFacing().GetRadian<32>();
						// 目标无朝向（格子/地面）：保持 facingU 初值（世界固定），不回退连线（同主圆处理）
					}
					break;
				case VectorOrigin::Source:
					{
						TechnoClass* pST = abstract_cast<TechnoClass*>(s._pSource);
						if (pST && !IsDeadOrInvisible(pST))
							facingU = pST->TurretFacing().GetRadian<32>();
						else if (s._pSource) { auto sp = s._pSource->GetCoords(); auto bp = pObject->GetCoords(); facingU = std::atan2(bp.Y-sp.Y, bp.X-sp.X); } // 非单位：回退连线
					}
					break;
				default: // FLH
					if (!IsEmpty(d.OriginOriginFLH.Get()))
					{
						double fy = d.OriginOriginFLH.Get().X, fx = d.OriginOriginFLH.Get().Y;
						facingU = std::atan2(fy, fx);
					}
					else if (pBullet) facingU = pBullet->Velocity.Magnitude() > 0 ? std::atan2(pBullet->Velocity.X, pBullet->Velocity.Y) : 0.0;
					else if (pTechno) facingU = pTechno->TurretFacing().GetRadian<32>();
					break;
				}
				double tiltU = originTerrainTilt; // 单位倾斜

				// 基础法向量（首帧锁定）→ 笛卡尔
				double bx = std::cos(s._baseOriginTilt) * std::cos(s._baseOriginFacing);
				double by = std::cos(s._baseOriginTilt) * std::sin(s._baseOriginFacing);
				double bz = std::sin(s._baseOriginTilt);
				// 1. 绕 Z 轴转 facingU（单位水平朝向）
				double cz = std::cos(facingU), sz = std::sin(facingU);
				double x1 = bx * cz - by * sz;
				double y1 = bx * sz + by * cz;
				double z1 = bz;
				// 2. 绕单位 L 轴 u=(-sinFU, cosFU, 0) 转 tiltU（Rodrigues）
				double ct = std::cos(tiltU), st = std::sin(tiltU);
				double ux = -sz, uy = cz, uz = 0.0;
				double dot = ux * x1 + uy * y1;      // u·n
				double cx = uy * z1 - uz * y1;       // u×n
				double cy = uz * x1 - ux * z1;
				double cz2 = ux * y1 - uy * x1;
				s._originMotion.normalX = x1 * ct + cx * st + ux * dot * (1.0 - ct);
				s._originMotion.normalY = y1 * ct + cy * st + uy * dot * (1.0 - ct);
				s._originMotion.normalZ = z1 * ct + cz2 * st;
			}
			// 每帧累加 Lissajous + 3D 法向量增量旋转
			s._originMotion.normalRotF += s._originMotion.lissajousStep;
			if (s._originMotion.normalStepF != 0.0 || s._originMotion.normalStepL != 0.0 || s._originMotion.normalStepH != 0.0)
			{
				RotateNormal3D(s._originMotion.normalX, s._originMotion.normalY, s._originMotion.normalZ,
					s._originMotion.normalStepF, s._originMotion.normalStepL, s._originMotion.normalStepH);
			}

			// OriginAllowCircleTilt：每帧从目标 Z 差更新大圆面倾斜
			if (d.OriginAllowCircleTilt.Get() && d.OriginOrigin.Get() == VectorOrigin::Target)
			{
				CoordStruct oc = baseCenter + s._originOffset;
				CoordStruct targetPos {};
				bool hasTargetPos = false;
				if (pBullet) { targetPos = pBullet->TargetCoords; hasTargetPos = true; }
				else if (pTechno && pTechno->Target) { targetPos = pTechno->Target->GetCoords(); hasTargetPos = true; }
				if (hasTargetPos)
				{
					double dx = oc.X - targetPos.X, dy = oc.Y - targetPos.Y, dz = oc.Z - targetPos.Z;
					double lenXY = std::sqrt(dx * dx + dy * dy);
					s._originTilt = (lenXY > 1e-6) ? std::atan2(dz, lenXY) : M_PI / 2.0;
				}
			}

			// 从法向量现算球坐标（段内 OriginIsNormalOnOrigin 每帧旋转结果已在 normalX/Y/Z）
			double oFacing = 0.0, oTilt = 0.0;
			{
				double lenXY = std::sqrt(s._originMotion.normalX * s._originMotion.normalX + s._originMotion.normalY * s._originMotion.normalY);
				oFacing = lenXY > 1e-6 ? std::atan2(s._originMotion.normalY, s._originMotion.normalX) : 0.0;
				oTilt = lenXY > 1e-6 ? std::atan2(s._originMotion.normalZ, lenXY) : (s._originMotion.normalZ > 0 ? M_PI / 2.0 : -M_PI / 2.0);
			}
			// OriginIsNormalOnOrigin=yes 时法向量已按单位倾斜每帧旋转，OriginAllowOriginTilt 不再叠加，避免重复
			oTilt += (d.OriginAllowOriginTilt.Get() && !d.OriginIsNormalOnOrigin.Get() ? originTerrainTilt : 0.0);
			// 3D 法向量旋转覆盖
			if (s._originMotion.normalStepF != 0.0 || s._originMotion.normalStepL != 0.0 || s._originMotion.normalStepH != 0.0)
			{
				double lenXY = std::sqrt(s._originMotion.normalX * s._originMotion.normalX + s._originMotion.normalY * s._originMotion.normalY);
				oFacing = lenXY > 1e-6 ? std::atan2(s._originMotion.normalY, s._originMotion.normalX) : 0.0;
				oTilt = lenXY > 1e-6 ? std::atan2(s._originMotion.normalZ, lenXY) : (s._originMotion.normalZ > 0 ? M_PI / 2.0 : -M_PI / 2.0);
			}
			DirStruct oFacingDir = Radians2Dir(oFacing);

			// 当前圆心绝对位置 = 基座 + 偏移
			CoordStruct originCenter = baseCenter + s._originOffset;

			CoordStruct disp;
			if (!IsEmpty(d.OriginMoveTo.Get()))
			{
				// MoveTo 模式：GrowRate 随帧数线性增长
				s._originMotion.angle += d.OriginAnglePerStep.Get();
				CoordStruct growOffset;
				growOffset = d.OriginGrowRate.Get() * s._originMotion.elapsed;
				disp = GetFLHAbsoluteOffset(d.OriginMoveTo.Get() + growOffset, Radians2Dir(oFacing + Math::deg2rad(s._originMotion.angle)));
			}
			else if (d.OriginReachTarget.Get() || d.OriginLinearSpeed.Get() >= 0 || !IsEmpty(d.OriginTargetFLH.Get()))
			{
				// Speed / ReachTarget
				if (s._originMotion.elapsed == 0)
				{
					s._originMotion.speed = d.OriginLinearSpeed.Get() >= 0 ? d.OriginLinearSpeed.Get() : (pTechno ? pTechno->GetTechnoType()->Speed : 40.0);
					s._originMotion.arcStartCenter = originCenter;
				}

				CoordStruct targetWorld = GetFLHAbsoluteCoords(baseCenter, d.OriginTargetFLH.Get() + s._originTargetOffset, oFacingDir);
				if (d.OriginReachTarget.Get())
				{
					int effectiveSteps = (duration - d.DisabledFrames.Get()) / s._effectiveTimeStep;
					if (effectiveSteps < 1) effectiveSteps = 1;
					int rem = effectiveSteps - s._movementFrames;
					if (rem <= 0)
					{
						disp = targetWorld - originCenter;
						s._originOffset += disp;
						circleCenter = baseCenter + s._originOffset;
						s._prevCircleCenter = circleCenter;
						out.Deactivate = true;
						goto skipOriginUpdate;
					}
					disp.X = (targetWorld.X - originCenter.X) / rem;
					disp.Y = (targetWorld.Y - originCenter.Y) / rem;
					disp.Z = (targetWorld.Z - originCenter.Z) / rem;
					if (s._originMotion.arcHeight != 0)
					{
						double t = static_cast<double>(s._movementFrames) / effectiveSteps;
						double arcOffset = CalcArcOffsetAt(static_cast<int>(s._originMotion.arcHeight), s._originMotion.arcPeakPercent, t);
						double baseX = s._originMotion.arcStartCenter.X + (targetWorld.X - s._originMotion.arcStartCenter.X) * t;
						double baseY = s._originMotion.arcStartCenter.Y + (targetWorld.Y - s._originMotion.arcStartCenter.Y) * t;
						double baseZ = s._originMotion.arcStartCenter.Z + (targetWorld.Z - s._originMotion.arcStartCenter.Z) * t;
						if (s._originMotion.arcRotation == 0.0)
						{
							disp.Z = static_cast<int>(baseZ + arcOffset) - originCenter.Z;
						}
						else
						{
							CoordStruct arcD{
								targetWorld.X - s._originMotion.arcStartCenter.X,
								targetWorld.Y - s._originMotion.arcStartCenter.Y,
								targetWorld.Z - s._originMotion.arcStartCenter.Z };
							ArcDelta3D ad = RotateArcDelta(arcD, s._originMotion.arcRotation, arcOffset);
							disp.X = static_cast<int>(baseX + ad.x) - originCenter.X;
							disp.Y = static_cast<int>(baseY + ad.y) - originCenter.Y;
							disp.Z = static_cast<int>(baseZ + ad.z) - originCenter.Z;
						}
					}
				}
				else
				{
					s._originMotion.speed += d.OriginAcceleration.Get();
					if (d.OriginMaxSpeed.Get() >= 0 && s._originMotion.speed > d.OriginMaxSpeed.Get()) s._originMotion.speed = d.OriginMaxSpeed.Get();
					if (d.OriginMinSpeed.Get() >= 0 && s._originMotion.speed < d.OriginMinSpeed.Get()) s._originMotion.speed = d.OriginMinSpeed.Get();
					int dx = targetWorld.X - originCenter.X, dy = targetWorld.Y - originCenter.Y, dz = targetWorld.Z - originCenter.Z;
					double dist = std::sqrt((double)dx*dx + dy*dy + dz*dz);
					if (dist < 1.0) disp = {};
					else if (d.OriginSpeedEndOnReach.Get() && s._originMotion.speed >= dist)
					{
						disp.X = dx; disp.Y = dy; disp.Z = dz;
						out.Deactivate = true;
					}
					else { double s_ = s._originMotion.speed / dist; disp.X = (int)(dx*s_); disp.Y = (int)(dy*s_); disp.Z = (int)(dz*s_); }

					// 弧高增量叠加（与 OriginReachTarget 一致，支持 ArcPeakPercent / ArcRotation）
					if (s._originMotion.arcHeight != 0 && dist >= 1.0)
					{
						if (s._originMotion.arcTotalDist < 0.0)
							s._originMotion.arcTotalDist = dist;
						double t = (s._originMotion.arcTotalDist > 1e-6) ? 1.0 - dist / s._originMotion.arcTotalDist : 0.0;
						if (t < 0.0) t = 0.0; else if (t > 1.0) t = 1.0;
						double arcThis = CalcArcOffsetAt(static_cast<int>(s._originMotion.arcHeight), s._originMotion.arcPeakPercent, t);
						double arcDelta = arcThis - s._originMotion.prevArcOffset;
						s._originMotion.prevArcOffset = arcThis;
						CoordStruct arcD{
							targetWorld.X - s._originMotion.arcStartCenter.X,
							targetWorld.Y - s._originMotion.arcStartCenter.Y,
							targetWorld.Z - s._originMotion.arcStartCenter.Z };
						ArcDelta3D ad = RotateArcDelta(arcD, s._originMotion.arcRotation, arcDelta);
						disp.X += static_cast<int>(ad.x);
						disp.Y += static_cast<int>(ad.y);
						disp.Z += static_cast<int>(ad.z);
					}
				}
			}
			else // Circle 模式
			{
				s._originMotion.circleRadius += d.OriginCircleRadiusGrow.Get();
				double tr = s._originMotion.circleRadius;
				if (d.OriginCircleMaxRadius.Get() > 0 && tr > d.OriginCircleMaxRadius.Get()) tr = d.OriginCircleMaxRadius.Get();
				if (d.OriginCircleMinRadius.Get() > 0 && tr < d.OriginCircleMinRadius.Get()) tr = d.OriginCircleMinRadius.Get();
				// 角步长：优先线速度/半径推算，否则用固定角速度
				double originAngleStep = d.OriginCircleAnglePerStep.Get();
				if (d.OriginCircleSpeed.Get() != 0 && tr > 0)
					originAngleStep = Math::rad2deg(d.OriginCircleSpeed.Get() / tr);
				// Lissajous>0: 累积大角旋转（增减边震荡），==0: 每帧仅增量旋转（平滑行星）
				s._originMotion.angle += originAngleStep;
				double r = d.OriginLissajous.Get() > 0.0 ? Math::deg2rad(s._originMotion.angle + s._originMotion.normalRotF) : Math::deg2rad(originAngleStep + s._originMotion.normalRotF);
				double ca = std::cos(r), sa = std::sin(r);
				// 当前圆心相对基座的偏移在 LH 平面投影
				double dx = (double)s._originOffset.X, dy = (double)s._originOffset.Y, dz = (double)s._originOffset.Z;
				double cf = std::cos(oFacing), sf = std::sin(oFacing), ct = std::cos(oTilt), st = std::sin(oTilt);
				double dL = dx*(-sf) + dy*cf;
				double dH = dx*(-cf*st) + dy*(-sf*st) + dz*ct;
				double cd = std::sqrt(dL*dL + dH*dH);
				// 圆心在基座上（偏移≈0），初始化到半径位置
				if (cd < 1.0 && tr > 0)
				{
					dL = tr; dH = 0; cd = tr;
				}
				else if (cd < 1.0) cd = 1.0;
				double rL = (dL/cd*tr*ca - dH/cd*tr*sa), rH = (dL/cd*tr*sa + dH/cd*tr*ca);
				// 新偏移（世界坐标）
				CoordStruct newOffset;
				newOffset.X = (int)(rL*(-sf) + rH*(-cf*st));
				newOffset.Y = (int)(rL*cf + rH*(-sf*st));
				newOffset.Z = (int)(rH*ct);
				disp.X = newOffset.X - s._originOffset.X;
				disp.Y = newOffset.Y - s._originOffset.Y;
				disp.Z = newOffset.Z - s._originOffset.Z;
			}
	skipOriginUpdate:
			s._originOffset += disp;
			circleCenter = baseCenter + s._originOffset;
			s._originMotion.elapsed++;
		}

		// 圆心位移叠加：Circle 模式追踪圆心→调整 currentPos
		CoordStruct centerDelta{ 0, 0, 0 };  // 初始化避免 C4701 警告
		bool useCenterTracking = false;
		if (s._prevCircleCenter.X || s._prevCircleCenter.Y || s._prevCircleCenter.Z)
		{
			centerDelta = circleCenter - s._prevCircleCenter;
			if (d.OriginLissajous.Get() <= 0.0 && (d.OriginCircleRadius.Get() >= 0 || d.OriginCircleSpeed.Get() != 0 || d.OriginLinearSpeed.Get() >= 0 || d.OriginCircleAnglePerStep.Get() != 0.0))
				useCenterTracking = true;
		}
		s._prevCircleCenter = circleCenter;

		// 圆上目标基于内部跟踪位置（非 currentPos），避免与 MoveTo 等 AE 的位移打架
		if (IsEmpty(s._circlePos))
			s._circlePos = currentPos;
		CoordStruct trackPos = s._circlePos;
		if (useCenterTracking)
		{
			trackPos.X += centerDelta.X;
			trackPos.Y += centerDelta.Y;
			trackPos.Z += centerDelta.Z;
		}
		double dx = static_cast<double>(trackPos.X - circleCenter.X);
		double dy = static_cast<double>(trackPos.Y - circleCenter.Y);
		double dz = static_cast<double>(trackPos.Z - circleCenter.Z);
		double currentDist;
		bool useTiltPlane = hasNormal || (d.AllowCircleTilt.Get() && effectiveTilt != 0.0);
		if (useTiltPlane)
		{
			// 倾斜圆面：把世界向量投影到圆面局部 LH 平面（L=水平切向，H=圆面"上"方向）
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
		if (currentDist < 1.0) currentDist = 1.0;

		// 动态半径：首帧初始化，每帧叠加增长率
		if (s._elapsedFrames == 0)
		{
			s._motion.circleRadius = static_cast<double>(d.CircleRadius.Get());
			if (s._motion.circleRadius <= 0.0)
				s._motion.circleRadius = currentDist;
			if (d.CircleRandomRadiusMax > d.CircleRandomRadiusMin)
				s._motion.circleRadius = Rnd(d.CircleRandomRadiusMin, d.CircleRandomRadiusMax);
		}
		s._motion.circleRadius += d.CircleRadiusGrow.Get();

		double targetRadius = s._motion.circleRadius;
		// 钳位
		if (d.CircleMaxRadius.Get() > 0 && targetRadius > d.CircleMaxRadius.Get())
			targetRadius = static_cast<double>(d.CircleMaxRadius.Get());
		if (d.CircleMinRadius.Get() > 0 && targetRadius < d.CircleMinRadius.Get())
			targetRadius = static_cast<double>(d.CircleMinRadius.Get());

		double rad = Math::deg2rad(angleStep + s._motion.normalRotF);
		double cosA = std::cos(rad), sinA = std::sin(rad);

		if (useTiltPlane)
		{
			// 倾斜圆面取点（与上方投影同一套正交基）
			double cosF = std::cos(effectiveFacing), sinF = std::sin(effectiveFacing);
			double cosT = std::cos(effectiveTilt), sinT = std::sin(effectiveTilt);
			double dL = dx * (-sinF) + dy * cosF;
			double dH = dx * (-cosF * sinT) + dy * (-sinF * sinT) + dz * cosT;
			double curDist = std::sqrt(dL * dL + dH * dH);
			if (curDist < 1.0) curDist = 1.0;
			double ndL = (dL / curDist * targetRadius);
			double ndH = (dH / curDist * targetRadius);
			double rL = ndL * cosA - ndH * sinA;
			double rH = ndL * sinA + ndH * cosA;
			out.MoveDisp.X = circleCenter.X + static_cast<int>(rL * (-sinF) + rH * (-cosF * sinT)) - s._circlePos.X;
			out.MoveDisp.Y = circleCenter.Y + static_cast<int>(rL * cosF + rH * (-sinF * sinT)) - s._circlePos.Y;
			out.MoveDisp.Z = circleCenter.Z + static_cast<int>(rH * cosT) - s._circlePos.Z;
		}
		else
		{
			// 传统 2D 圆面（XY 平面）
			double ndx = (dx / currentDist * targetRadius);
			double ndy = (dy / currentDist * targetRadius);
			double rx = ndx * cosA - ndy * sinA;
			double ry = ndx * sinA + ndy * cosA;
			out.MoveDisp.X = circleCenter.X + static_cast<int>(rx) - s._circlePos.X;
			out.MoveDisp.Y = circleCenter.Y + static_cast<int>(ry) - s._circlePos.Y;
			out.MoveDisp.Z = IsEmpty(d.CircleOrigin.Get()) && IsEmpty(d.OriginFLH.Get())
				? 0 : circleCenter.Z - s._circlePos.Z;  // 有显式高度指定时拉 Z，否则维持抛射体自身高度
		}
		s._circlePos.X += out.MoveDisp.X;
		s._circlePos.Y += out.MoveDisp.Y;
		s._circlePos.Z += out.MoveDisp.Z;
		out.Force = true;

		// 到达边界时结束
		if (d.CircleEndOnMaxRadius.Get() && d.CircleMaxRadius.Get() > 0
			&& s._motion.circleRadius >= d.CircleMaxRadius.Get())
		{
			out.Deactivate = true;
		}
		if (d.CircleEndOnMinRadius.Get() && d.CircleMinRadius.Get() > 0
			&& s._motion.circleRadius <= d.CircleMinRadius.Get())
		{
			out.Deactivate = true;
		}

		s.AdvanceFrame();
		return true;
	}

	// ------------------------------------------------------------------------
	// 主运动：MoveTo 模式（纯 FLH 位移 + GrowRate + AnglePerStep 自旋）
	// ------------------------------------------------------------------------
	static bool StepMoveTo(VectorRevibedState& s, const VectorRevibedData& d,
		BulletClass* pBullet, TechnoClass* pTechno, ObjectClass* pObject,
		const CoordStruct& currentPos, DirStruct mainFacingDir, double effectiveTilt,
		VectorRevibedResult& out)
	{
		DirStruct moveDir = mainFacingDir;
		double useCosT = 1.0, useSinT = 0.0;
		bool hasTilt = (effectiveTilt != 0.0);

		// Origin=Target：F 轴应以 抛射体→目标 连线为准（含 Z 落差），而非全局的 target→抛射体
		if (d.Origin.Get() == VectorOrigin::Target)
		{
			CoordStruct tgt {};
			bool hasTgt = false;
			if (pBullet && pBullet->Target)
				{ tgt = pBullet->Target->GetCoords(); hasTgt = true; }
			else if (pBullet)
				{ tgt = pBullet->TargetCoords; hasTgt = true; }
			else if (pTechno && pTechno->Target)
				{ tgt = pTechno->Target->GetCoords(); hasTgt = true; }
			if (hasTgt)
			{
				double tdx = static_cast<double>(tgt.X - currentPos.X);
				double tdy = static_cast<double>(tgt.Y - currentPos.Y);
				double tdz = static_cast<double>(tgt.Z - currentPos.Z);
				double tLenXY = std::sqrt(tdx * tdx + tdy * tdy);
				if (tLenXY > 1e-6)
				{
					// 与 AutoWeapon IsOnTarget 同款：Point2Dir 内部处理 RA2 坐标系（裸 atan2+Radians2Dir 有 90° 偏置）
					moveDir = Point2Dir(currentPos, tgt); // 抛射体→目标 连线方向
					if (d.AllowCircleTilt.Get())
					{
						double tLen3D = std::sqrt(tdx * tdx + tdy * tdy + tdz * tdz);
						useCosT = tLenXY / tLen3D;
						useSinT = tdz / tLen3D;
						hasTilt = true;
					}
				}
			}
		}

		if (d.AnglePerStep.Get() != 0.0)
		{
			if (s._elapsedFrames == 0)
				s._motion.angle = 0.0;
			s._motion.angle += d.AnglePerStep.Get();
			// SetRadian 与 GetRadian 互逆（Radians2Dir 往返存在 90° 偏置，不能用）
			moveDir.SetRadian<32>(moveDir.GetRadian<32>() + Math::deg2rad(s._motion.angle));
		}

		CoordStruct grow = { static_cast<int>(d.GrowRate.Get().X * s._movementFrames),
			static_cast<int>(d.GrowRate.Get().Y * s._movementFrames),
			static_cast<int>(d.GrowRate.Get().Z * s._movementFrames) };
		CoordStruct moveFlh = d.MoveTo.Get() + grow;

		if (hasTilt)
		{
			double mf = moveDir.GetRadian<32>();
			double cosF = std::cos(mf), sinF = std::sin(mf);
			double cosT = (d.Origin.Get() == VectorOrigin::Target && d.AllowCircleTilt.Get())
				? useCosT : std::cos(effectiveTilt);
			double sinT = (d.Origin.Get() == VectorOrigin::Target && d.AllowCircleTilt.Get())
				? useSinT : std::sin(effectiveTilt);
			double F = static_cast<double>(moveFlh.X);
			double L = static_cast<double>(moveFlh.Y);
			double H = static_cast<double>(moveFlh.Z);
			out.MoveDisp.X = static_cast<int>(F * cosF * cosT + L * (-sinF) + H * (-cosF * sinT));
			out.MoveDisp.Y = static_cast<int>(F * sinF * cosT + L * cosF + H * (-sinF * sinT));
			out.MoveDisp.Z = static_cast<int>(F * sinT + H * cosT);
		}
		else
		{
			out.MoveDisp = GetFLHAbsoluteOffset(moveFlh, moveDir);
		}

		out.Force = true;

		s.AdvanceFrame();
		return true;
	}

	// ------------------------------------------------------------------------
	// 主运动：TargetFLH 模式（ReachTarget 均分到达 / Speed 直线追踪+影子弧高）
	// ------------------------------------------------------------------------
	static bool StepTargetFLH(VectorRevibedState& s, const VectorRevibedData& d,
		BulletClass* pBullet, TechnoClass* pTechno, ObjectClass* pObject,
		const CoordStruct& currentPos, const CoordStruct& originPos, DirStruct mainFacingDir,
		VectorRevibedResult& out)
	{
		// --- 目标世界坐标 ---
		CoordStruct frameTargetFlh;
		// TargetOffsetNormal 世界固定：偏移已由 LockFacing 转成世界坐标，叠加在旋转后的 TargetFLH 上（不随 F 轴转）
		bool targetOffsetWorld = !d.IsNormalOnOrigin.Get() && !IsEmpty(d.TargetOffsetNormal.Get());
		frameTargetFlh.X = d.TargetFLH.Get().X + (targetOffsetWorld ? 0 : s._randomTargetOffset.X);
		frameTargetFlh.Y = d.TargetFLH.Get().Y + (targetOffsetWorld ? 0 : s._randomTargetOffset.Y);
		frameTargetFlh.Z = d.TargetFLH.Get().Z + (targetOffsetWorld ? 0 : s._randomTargetOffset.Z);

		// TargetFLH → 世界坐标：AutoWeapon 同款管线
		// 坐标系统一：矩阵偏移（含 IsOnTurret 炮塔/车身）+ NoUpdate 控制的计算点 originPos
		CoordStruct frameTarget;
		bool isOnTurret = !d.OriginIsOnBody.Get(); // AutoWeapon 语义：yes=炮塔指向，no=车身指向
		switch (d.Origin.Get())
		{
		case VectorOrigin::Launcher:
			{
				TechnoClass* pLT = abstract_cast<TechnoClass*>(s._pLauncher);
				if (pLT && !IsDeadOrInvisible(pLT))
				{
					// 已知不一致（Kratos 原样保留）：OriginIsOnWorld=yes 时此路径仍用发射者炮塔矩阵旋转 FLH
					// 实际中 OriginIsOnWorld + Origin=Launcher 组合过于怪异，不做处理。
					// PhobosAI 用现成 TechnoExt::GetFLHAbsoluteCoords（Phobos 官方矩阵管线，行为等价）
					CoordStruct mtxPos = TechnoExt::GetFLHAbsoluteCoords(pLT, frameTargetFlh, isOnTurret);
					frameTarget = originPos + (mtxPos - pLT->GetCoords());
				}
				else
					frameTarget = GetFLHAbsoluteCoords(originPos, frameTargetFlh, mainFacingDir);
			}
			break;
		case VectorOrigin::Self:
			if (d.OriginIsOnWorld.Get())
				frameTarget = GetFLHAbsoluteCoords(originPos, frameTargetFlh, DirStruct{}); // 世界坐标系，无视倾斜
			else if (pTechno)
			{
				// AutoWeapon 同款：Locomotor 矩阵 + TurretOffset + 炮塔旋转角
				CoordStruct mtxPos = TechnoExt::GetFLHAbsoluteCoords(pTechno, frameTargetFlh, isOnTurret);
				frameTarget = originPos + (mtxPos - pTechno->GetCoords());
			}
			else
				frameTarget = GetFLHAbsoluteCoords(originPos, frameTargetFlh, mainFacingDir);
			break;
		default: // Target / Source
			frameTarget = GetFLHAbsoluteCoords(originPos, frameTargetFlh, mainFacingDir);
			break;
		}

		// TargetOffsetNormal 世界固定：偏移（世界坐标）叠加在旋转后的 TargetFLH 上，不随 F 轴转
		if (targetOffsetWorld)
		{
			frameTarget.X += s._randomTargetOffset.X;
			frameTarget.Y += s._randomTargetOffset.Y;
			frameTarget.Z += s._randomTargetOffset.Z;
		}

		CoordStruct dirVec;
		dirVec.X = frameTarget.X - currentPos.X;
		dirVec.Y = frameTarget.Y - currentPos.Y;
		dirVec.Z = frameTarget.Z - currentPos.Z;
		double dirLen = std::sqrt(static_cast<double>(dirVec.X * dirVec.X + dirVec.Y * dirVec.Y + dirVec.Z * dirVec.Z));

		// 同步 Rocket loco 俯仰角：引擎自主移动姿态与 Vector 移动向量一致，
		// 防止 Vector 结束后引擎按错误的 Pitch 继续飞行导致命中偏移（Spawn 导弹用 RocketLocomotionClass）
		if (pTechno)
		{
			// locomotion_cast：官方 YRpp 的 Locomotor 是 _com_ptr_t（ILocomotionPtr），
			// 用 YRpp/LocomotionClass.h 的工具转具体类（RocketLocomotionClass）
			if (RocketLocomotionClass* rLoco = locomotion_cast<RocketLocomotionClass*>(
				abstract_cast<FootClass*, true>(pTechno)->Locomotor))
			{
				double lenXY = std::sqrt(static_cast<double>(dirVec.X * dirVec.X + dirVec.Y * dirVec.Y));
				rLoco->CurrentPitch = static_cast<float>(std::atan2(dirVec.Z, lenXY));
			}
		}

		CoordStruct resultDisp{ 0, 0, 0 };

		// ========================================================================
		// 模式 2: ReachTarget（剩余帧数强制到达）
		// ========================================================================
		if (d.ReachTarget.Get() && s._totalDuration > 0)
		{
			int effectiveDuration = s._totalDuration - d.DisabledFrames.Get();
			if (effectiveDuration < 1) effectiveDuration = 1;
			int remainingFrames = effectiveDuration - s._movementFrames + 1;
			if (d.ReachTargetEarlyEnd.Get() > 0 && d.ReachTargetEarlyEnd.Get() < effectiveDuration
				&& remainingFrames <= d.ReachTargetEarlyEnd.Get())
			{
				out.Deactivate = true;
				s.AdvanceFrame();
				return true;
			}
			if (dirLen > 1e-6)
			{
				// AE 根基缺陷：Duration=N 实际只执行 N-1 个运动帧（末帧 AE 已移除）
				// 轨迹按"实际帧数 = 总帧数 - 1"均分，保证 AE 实际删除的那一帧恰好到位
				double adjustedSpeed = dirLen / (remainingFrames > 1 ? remainingFrames - 1 : 1);
				resultDisp.X = static_cast<int>(dirVec.X / dirLen * adjustedSpeed);
				resultDisp.Y = static_cast<int>(dirVec.Y / dirLen * adjustedSpeed);
				resultDisp.Z = static_cast<int>(dirVec.Z / dirLen * adjustedSpeed);

				// 抛物线弧高（Speed 模式影子算法：增量叠加，t 跟随实际路程，目标移动自动校准）
				if (s._motion.arcHeight != 0)
				{
					// 影子沿 frameTarget 方向推进 adjustedSpeed（与直线均分同步）
					double ux = dirVec.X / dirLen, uy = dirVec.Y / dirLen, uz = dirVec.Z / dirLen;
					s._motion.shadowX += ux * adjustedSpeed;
					s._motion.shadowY += uy * adjustedSpeed;
					s._motion.shadowZ += uz * adjustedSpeed;
					s._motion.shadowTraveled += adjustedSpeed;

					// 剩余影子距离（3D）
					double sdx = frameTarget.X - s._motion.shadowX;
					double sdy = frameTarget.Y - s._motion.shadowY;
					double sdz = frameTarget.Z - s._motion.shadowZ;
					double shadowDist = std::sqrt(sdx * sdx + sdy * sdy + sdz * sdz);

					// t = 已走路程 / 总路程（动态更新，目标移动时自动调整）
					double total = s._motion.shadowTraveled + shadowDist;
					double t = (total > 1e-6) ? s._motion.shadowTraveled / total : 0.0;
					if (t < 0.0) t = 0.0; else if (t > 1.0) t = 1.0;

					// 弧高增量叠加（不覆盖直线位移）
					double arcOffset = CalcArcOffsetAt(static_cast<int>(s._motion.arcHeight), s._motion.arcPeakPercent, t);
					double arcDelta = arcOffset - s._motion.prevArcOffset;
					s._motion.prevArcOffset = arcOffset;

					CoordStruct arcD{
						frameTarget.X - s._initialLocation.X,
						frameTarget.Y - s._initialLocation.Y,
						frameTarget.Z - s._initialLocation.Z };
					ArcDelta3D ad = RotateArcDelta(arcD, s._motion.arcRotation, arcDelta);
					resultDisp.X += static_cast<int>(ad.x);
					resultDisp.Y += static_cast<int>(ad.y);
					resultDisp.Z += static_cast<int>(ad.z);
				}
			}
			out.MoveDisp = resultDisp;
			s.AdvanceFrame();
			return true;
		}

		// ========================================================================
		// 模式 5: Speed（直线追踪 + 加速度 + 影子坐标弧高）
		// 影子沿 _shadowPos→frameTarget 方向推进，不受弧高Z偏移污染
		// ========================================================================
		if (d.LinearSpeed.Get() >= 0)
		{
			double speed = s._motion.speed;

			// 加速度
			if (d.Acceleration.Get() != 0)
			{
				speed += d.Acceleration.Get() * s._elapsedFrames;
			}

			// 钳位
			if (d.MinSpeed.Get() >= 0 && speed < d.MinSpeed.Get())
				speed = static_cast<double>(d.MinSpeed.Get());
			if (d.MaxSpeed.Get() >= 0 && speed > d.MaxSpeed.Get())
				speed = static_cast<double>(d.MaxSpeed.Get());

			// 影子系统：弧线（影子坐标弧高）不依赖 SpeedEndOnReach

			double sdx = frameTarget.X - s._motion.shadowX;
			double sdy = frameTarget.Y - s._motion.shadowY;
			double sdz, shadowDist;
			if (s._motion.arcHeight != 0)
			{
				sdz = frameTarget.Z - s._motion.shadowZ;
				shadowDist = std::sqrt(sdx * sdx + sdy * sdy + sdz * sdz);
			}
			else
			{
				// 无弧线：仅 XY 距离，避免影子 Z 追踪目标导致 shadowDist 虚小
				sdz = 0.0;
				shadowDist = std::sqrt(sdx * sdx + sdy * sdy);
			}

			// SpeedEndOnReach：瞬移到目标位置，引擎正常到达检测会自然引爆
			// 到达判定用真实 3D 距离（含 Z），避免高空俯冲时 XY 先对齐导致误判到达
			if (d.SpeedEndOnReach.Get())
			{
				double realDX = frameTarget.X - currentPos.X;
				double realDY = frameTarget.Y - currentPos.Y;
				double realDZ = frameTarget.Z - currentPos.Z;
				double realDist = std::sqrt(realDX * realDX + realDY * realDY + realDZ * realDZ);

				if (realDist <= speed)
				{
					// 强制挪移：到达帧直接把对象坐标设为目标格子坐标（完全重合），消除到位抖动
					// PhobosAI 并存语义：TeleportTo 由调用方在"唯一活跃"时应用；
					// 并存时忽略瞬移（该 Vector 结束，引擎接管）
					out.TeleportTo = frameTarget;
					out.MoveDisp = { 0, 0, 0 };
					out.Force = false;
					out.Deactivate = true;
					s.AdvanceFrame();
					return true;
				}
			}

			if (shadowDist > 1e-6)
			{
				// 影子沿 shadow→target 方向推进，步长钳位到剩余距离：
				// 末段 speed > 剩余距离时若仍推进满 speed 会越过目标，sdx 变号导致来回振荡（原地抽搐）
				double step = (speed < shadowDist) ? speed : shadowDist;
				double sInv = 1.0 / shadowDist;
				double shadowStepX = sdx * sInv * step;
				double shadowStepY = sdy * sInv * step;
				double shadowStepZ = 0.0;
				s._motion.shadowX += shadowStepX;
				s._motion.shadowY += shadowStepY;
				if (s._motion.arcHeight != 0)
				{
					shadowStepZ = sdz * sInv * step;
					s._motion.shadowZ += shadowStepZ;
				}
				// 无弧线：_shadowPosZ 不变（始终 = _initialLocation.Z），Z 由 t 插值
				s._motion.shadowTraveled += step;

				// 重新计算影子距离（影子已移动）
				sdx = frameTarget.X - s._motion.shadowX;
				sdy = frameTarget.Y - s._motion.shadowY;
				if (s._motion.arcHeight != 0)
				{
					sdz = frameTarget.Z - s._motion.shadowZ;
					shadowDist = std::sqrt(sdx * sdx + sdy * sdy + sdz * sdz);
				}
				else
				{
					shadowDist = std::sqrt(sdx * sdx + sdy * sdy);
				}

				// t = 已走路程 / 总路程（动态更新，目标移动时自动调整）
				double total = s._motion.shadowTraveled + shadowDist;
				double t = (total > 1e-6) ? s._motion.shadowTraveled / total : 0.0;
				if (t < 0.0) t = 0.0; else if (t > 1.0) t = 1.0;

				// 实际位移：影子步长（XY）
				resultDisp.X = static_cast<int>(shadowStepX);
				resultDisp.Y = static_cast<int>(shadowStepY);

				if (s._motion.arcHeight != 0)
				{
					// 有弧线：Z 用影子增量 + 弧高增量叠加
					resultDisp.Z = static_cast<int>(shadowStepZ);
				}
				else
				{
					// 无弧线：Z 从抛射体起始高度 lerp 到目标高度
					double targetZ = s._motion.shadowZ + (frameTarget.Z - s._motion.shadowZ) * t;
					resultDisp.Z = static_cast<int>(targetZ - currentPos.Z);
				}

				if (s._motion.arcHeight != 0)
				{
					double arcOffset = CalcArcOffsetAt(static_cast<int>(s._motion.arcHeight), s._motion.arcPeakPercent, t);
					double arcDelta = arcOffset - s._motion.prevArcOffset;
					s._motion.prevArcOffset = arcOffset;

					CoordStruct arcD{
						frameTarget.X - s._initialLocation.X,
						frameTarget.Y - s._initialLocation.Y,
						frameTarget.Z - s._initialLocation.Z };
					ArcDelta3D ad = RotateArcDelta(arcD, s._motion.arcRotation, arcDelta);
					resultDisp.X += static_cast<int>(ad.x);
					resultDisp.Y += static_cast<int>(ad.y);
					resultDisp.Z += static_cast<int>(ad.z);
				}
			}
			out.MoveDisp = resultDisp;
			s.AdvanceFrame();
			return true;
		}

		// 没命中任何模式，返回空
		s.AdvanceFrame();
		return true;
	}

} // namespace VectorRevibedImpl

// ============================================================================
// Step 主函数（对应 Kratos GetVectorResult；编排 + 朝向/Origin 计算 + 模式分发）
// ============================================================================
void VectorRevibedAI_Step(VectorRevibedState& s, const VectorRevibedData& d,
	ObjectClass* pObject, ObjectClass* pLauncher, ObjectClass* pSource, int duration,
	VectorRevibedResult& out)
{
	using namespace VectorRevibedImpl;

	BulletClass* pBullet = abstract_cast<BulletClass*>(pObject);
	TechnoClass* pTechno = abstract_cast<TechnoClass*>(pObject);

	// 首帧快照（仅一次，供弧高计算等）
	if (s._elapsedFrames == 0)
		s._initialLocation = pObject->GetCoords();

	// （Kratos InitialDelay/_started 检查删除：PhobosAI 挂载即启动）
	// （Kratos CacheTargetNow 每帧刷新删除：无 TechnoStatus 缓存）

	// 目标坐标固化：Init 未锁定（Pending）时补读，读到即锁定到 _initialOriginPos，
	// 防止引擎后续清空（目标死亡/管理器清空）导致 frameTarget 失效
	if (d.Origin.Get() == VectorOrigin::Target
		&& IsEmpty(s._initialOriginPos) && pTechno)
	{
		CoordStruct targetPos{};
		bool got = false;
		if (!got && TryGetKamikazeTarget(pTechno, targetPos)) got = true;
		if (!got && TryGetSpawnManagerTarget(pTechno, targetPos)) got = true;
		if (!got && pTechno->Target) { targetPos = pTechno->Target->GetCoords(); got = true; }
		if (got)
		{
			s._initialOriginPos = targetPos;
		}
	}

	// Force 必须在闸门之前设，确保非运动帧也走 SetLocation（Freeze 等效）
	out.Force = d.Force.Get();
	out.AllowRotateUnit = d.SyncFacing.Get(); // 成熟机制：单位端同步朝向，删改前确认

	// Circle 预初始化：在 DisabledFrames 冻结前完成，保证首帧后参数可用
	if (s._elapsedFrames == 0)
	{
		s._motion.circleSpeed = static_cast<double>(d.CircleSpeed.Get());
		if (s._motion.circleSpeed <= 0.0)
		{
			if (pBullet)
				s._motion.circleSpeed = pBullet->Speed;
			else if (pTechno)
				s._motion.circleSpeed = pTechno->GetTechnoType()->Speed;
		}
		s._motion.circleAngle = d.CircleAnglePerStep.Get();
		if (d.CircleRandomAngleMax > d.CircleRandomAngleMin)
		{
			if (d.CircleRandomAngleMax2 > d.CircleRandomAngleMin2 && Rnd(0, 1))
				s._motion.circleAngle = d.CircleRandomAngleMin2 + (d.CircleRandomAngleMax2 - d.CircleRandomAngleMin2) * RndD();
			else
				s._motion.circleAngle = d.CircleRandomAngleMin + (d.CircleRandomAngleMax - d.CircleRandomAngleMin) * RndD();
		}
		s._motion.circleRadius = static_cast<double>(d.CircleRadius.Get());
		if (d.CircleRandomRadiusMax > d.CircleRandomRadiusMin)
			s._motion.circleRadius = Rnd(d.CircleRandomRadiusMin, d.CircleRandomRadiusMax);
	}

	// DisabledFrames：首帧快照后冻结，不阻塞其他 AE，不计入运动时间
	if (s._elapsedFrames < d.DisabledFrames.Get())
	{
		out.MoveDisp = { 0, 0, 0 };
		s.AdvanceFrame();
		return;
	}

	// ========================================================================
	// Freeze — 成熟机制，别乱动（Kratos 原样：本分支不 AdvanceFrame）
	// ========================================================================
	if (d.Freeze.Get())
	{
		out.Freeze = true;
		out.Force = true;  // 抛射体 Freeze 必须 Force，否则引擎检测"卡住"自爆
		if (IsEmpty(out.FrozenPos))
			out.FrozenPos = s._initialLocation;
		CoordStruct currentPos = pObject->GetCoords();
		out.MoveDisp = out.FrozenPos - currentPos;
		return;
	}

	// ========================================================================
	// TimeStep 闸门
	// ========================================================================
	if (!s.ShouldMoveThisFrame())
	{
		s._moveFrame++;
		return;
	}
	s._movementFrames++;

	s._motion.normalRotF += s._motion.lissajousStep;
	// 3D 法向量增量旋转（绕世界 F=Y / L=X / H=Z 轴，正速度=顺时针）
	if (s._motion.normalStepF != 0.0 || s._motion.normalStepL != 0.0 || s._motion.normalStepH != 0.0)
	{
		RotateNormal3D(s._motion.normalX, s._motion.normalY, s._motion.normalZ,
			s._motion.normalStepF, s._motion.normalStepL, s._motion.normalStepH);
	}

	CoordStruct currentPos = pObject->GetCoords();

	// ========================================================================
	// 动态 F 轴：非 NoUpdate 时每帧根据当前坐标重新计算 FLH 朝向
	// ========================================================================

	// originTerrainTilt：Origin 单位倾斜角（AngleRotatedForwards 动态倾斜优先，否则地形采样）。
	// 供两处使用：AllowOriginTilt 的 CircleOrigin FLH 旋转、大圆 OriginAllowOriginTilt 的 oTilt 叠加、
	// IsNormalOnOrigin 的法向量随单位转动。
	double originTerrainTilt = 0.0;
	bool hasCircleForTilt = d.CircleRadius.Get() > 0 || d.CircleAnglePerStep.Get() > 0.0
		|| (d.CircleRandomRadiusMax > d.CircleRandomRadiusMin)
		|| (d.CircleRandomAngleMax > d.CircleRandomAngleMin);
	if ((d.AllowOriginTilt.Get() || d.OriginAllowOriginTilt.Get() || d.IsNormalOnOrigin.Get()) && hasCircleForTilt && !d.OriginIsOnWorld.Get())
	{
		TechnoClass* pOriginTechno = nullptr;
		switch (d.Origin.Get())
		{
		case VectorOrigin::Target:
			if (pBullet && pBullet->Target)
				pOriginTechno = abstract_cast<TechnoClass*>(pBullet->Target);
			else if (pTechno && pTechno->Target)
				pOriginTechno = abstract_cast<TechnoClass*>(pTechno->Target);
			break;
		case VectorOrigin::Source:
			pOriginTechno = abstract_cast<TechnoClass*>(s._pSource);
			break;
		case VectorOrigin::Launcher:
			pOriginTechno = abstract_cast<TechnoClass*>(s._pLauncher);
			break;
		case VectorOrigin::Self:
			pOriginTechno = pTechno;
			break;
		}
		if (pOriginTechno && !IsDeadOrInvisible(pOriginTechno))
		{
			// 优先用引擎动态倾斜（Rocker等），为 0 时从地形采样
			originTerrainTilt = pOriginTechno->AngleRotatedForwards;
			if (std::abs(originTerrainTilt) < 1e-6)
			{
				CoordStruct originCoord = pOriginTechno->GetCoords();
				double unitFacing = pOriginTechno->PrimaryFacing.Current().GetRadian<32>();
				double cosF = std::cos(unitFacing), sinF = std::sin(unitFacing);
				Point2D frontPt = { originCoord.X + static_cast<int>(128.0 * cosF), originCoord.Y + static_cast<int>(128.0 * sinF) };
				Point2D backPt  = { originCoord.X - static_cast<int>(128.0 * cosF), originCoord.Y - static_cast<int>(128.0 * sinF) };
				int hFront = MapClass::Instance.GetCellFloorHeight({frontPt.X, frontPt.Y, 0});
				int hBack  = MapClass::Instance.GetCellFloorHeight({backPt.X, backPt.Y, 0});
				double dz = static_cast<double>(hFront - hBack);
				double dxy = 256.0;
				originTerrainTilt = (dxy > 1e-6) ? std::atan2(dz, dxy) : 0.0;
			}
		}
	}

	double effectiveFacing = s._facingRad;
	double effectiveTilt = s._tiltRad;
	DirStruct mainFacingDir = Radians2Dir(effectiveFacing);
	// Target/Source/Launcher/Self：统一用 Init 存的 DirStruct 作基础朝向。
	// Radians2Dir(GetRadian()) 往返存在 90° 偏置（BINARY_ANGLE_MAGIC 为负），
	// NoUpdate=no 时误走该路径导致坐标系旋转。NoUpdate 只决定原点是否动态刷新，不影响坐标系计算。
	bool hasNormal = !IsEmpty(d.NormalVector.Get())
		|| d.NormalRandomF.Get().Y > d.NormalRandomF.Get().X
		|| d.NormalRandomL.Get().Y > d.NormalRandomL.Get().X
		|| d.NormalRandomH.Get().Y > d.NormalRandomH.Get().X;
	if (!hasNormal && (d.Origin.Get() == VectorOrigin::Target
		|| d.Origin.Get() == VectorOrigin::Source
		|| d.Origin.Get() == VectorOrigin::Launcher
		|| d.Origin.Get() == VectorOrigin::Self))
	{
		mainFacingDir = s._facingDir;
		effectiveFacing = s._facingDir.GetRadian<32>();
	}

	// OriginIsOnWorld：锁定世界坐标系（朝北），覆盖 Origin 朝向
	if (d.OriginIsOnWorld.Get())
	{
		mainFacingDir = DirStruct{};
		effectiveFacing = 0.0;
		effectiveTilt = 0.0;
	}

	// 统一朝向算法：NoUpdate 只切换计算点，不切换坐标系/朝向算法
	if (!hasNormal && !d.AllowOriginTilt.Get() && !d.OriginIsOnWorld.Get())
	{
		switch (d.Origin.Get())
		{
		case VectorOrigin::Source:
			// 计算点：NoUpdate=yes 用锁定值，no 每帧刷新（三态跟踪：死亡冻结）
			TrackOriginCoord(s._pSource, d.OriginNoUpdate.Get(), s._initialOriginPos);
			if (!IsEmpty(s._initialOriginPos))
			{
				if (d.IsOnOrigin)
				{
					TechnoClass* pSourceTechno = abstract_cast<TechnoClass*>(s._pSource);
					if (pSourceTechno && !IsDeadOrInvisible(pSourceTechno))
					{
						mainFacingDir = d.OriginIsOnBody.Get()
							? pSourceTechno->PrimaryFacing.Current()
							: pSourceTechno->TurretFacing();
						effectiveFacing = mainFacingDir.GetRadian<32>();
						break;
					}
					// 来源无朝向：回退连线
				}
				// 来源活着或已死亡：都用快照算朝向（死亡后冻结指向死亡点）
				mainFacingDir = Point2Dir(s._initialOriginPos, currentPos);
				effectiveFacing = mainFacingDir.GetRadian<32>();
				double dx = currentPos.X - s._initialOriginPos.X;
				double dy = currentPos.Y - s._initialOriginPos.Y;
				double dz = currentPos.Z - s._initialOriginPos.Z;
				double lenXY = std::sqrt(dx*dx + dy*dy);
				effectiveTilt = (lenXY > 1e-6 && d.AllowCircleTilt.Get()) ? std::atan2(dz, lenXY) : 0.0;
			}
			break;
		case VectorOrigin::Target:
		{
			bool isGround = (pBullet && !abstract_cast<TechnoClass*>(pBullet->Target));
			if (isGround && s._movementFrames > 1)
				break;
			// 计算点：默认锁定值起步，NoUpdate=no 才走引擎链动态获取
			CoordStruct targetPos = s._initialOriginPos;
			bool gotTarget = !IsEmpty(targetPos);
			if (!gotTarget && !d.OriginNoUpdate.Get())
			{
				gotTarget = GetTargetPosFromChain(pBullet, pTechno, targetPos);
			}
			if (gotTarget)
				s._initialOriginPos = targetPos; // 跟随：更新锁定值
			else if (IsEmpty(targetPos))
				break; // 从未有过目标：保持朝向
			if (d.IsOnOrigin)
			{
				AbstractClass* pTgt = pBullet ? pBullet->Target : (pTechno ? pTechno->Target : nullptr);
				TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTgt);
				if (pTargetTechno && !IsDeadOrInvisible(pTargetTechno))
				{
					mainFacingDir = d.OriginIsOnBody.Get()
						? pTargetTechno->PrimaryFacing.Current()
						: pTargetTechno->TurretFacing();
					effectiveFacing = mainFacingDir.GetRadian<32>();
					break;
				}
				// 目标无朝向（格子）：回退连线
			}
			mainFacingDir = Point2Dir(targetPos, currentPos);
			effectiveFacing = mainFacingDir.GetRadian<32>();
			double dx = currentPos.X - targetPos.X, dy = currentPos.Y - targetPos.Y, dz = currentPos.Z - targetPos.Z;
			double lenXY = std::sqrt(dx*dx + dy*dy);
			effectiveTilt = (lenXY > 1e-6 && d.AllowCircleTilt.Get()) ? std::atan2(dz, lenXY) : 0.0;
		}
			break;

		case VectorOrigin::Self:
			if (pTechno)
			{
				mainFacingDir = d.OriginIsOnBody.Get()
					? pTechno->PrimaryFacing.Current()
					: pTechno->TurretFacing();
				effectiveFacing = mainFacingDir.GetRadian<32>();
			}
			else if (pBullet)
			{
				mainFacingDir = Facing(pBullet, currentPos);
				effectiveFacing = mainFacingDir.GetRadian<32>();
			}
			break;

		case VectorOrigin::Launcher:
			if (s._pLauncher && !IsDeadOrInvisible(s._pLauncher))
			{
				TechnoClass* pLauncherTechno = abstract_cast<TechnoClass*>(s._pLauncher);
				if (pLauncherTechno)
				{
					if (d.IsOnOrigin)
					{
						mainFacingDir = d.OriginIsOnBody.Get()
							? pLauncherTechno->PrimaryFacing.Current()
							: pLauncherTechno->TurretFacing();
						effectiveFacing = mainFacingDir.GetRadian<32>();
					}
					else
					{
						// 发射者→弹体连线
						mainFacingDir = Point2Dir(pLauncherTechno->GetCoords(), currentPos);
						effectiveFacing = mainFacingDir.GetRadian<32>();
					}
				}
			}
			break;
		}
	}

	// IsNormalOnOrigin：圆面法向量随 Origin 单位转动（facing + tilt 全跟随）
	// 基础法向量 = Init 锁定的球坐标，每帧按单位朝向（facingU）+ 单位倾斜（originTerrainTilt）转动：
	//   1. 绕 Z 轴转 facingU（单位水平朝向）
	//   2. 绕单位 L 轴 u=(-sinFU, cosFU, 0) 转 tiltU（Rodrigues）
	if (d.IsNormalOnOrigin.Get() && !d.OriginIsOnWorld.Get())
	{
		double facingU = effectiveFacing;
		switch (d.Origin.Get())
		{
		case VectorOrigin::Launcher:
			{
				TechnoClass* pLT = abstract_cast<TechnoClass*>(s._pLauncher);
				if (pLT && !IsDeadOrInvisible(pLT))
					facingU = (d.OriginIsOnBody.Get() ? pLT->PrimaryFacing.Current() : pLT->TurretFacing()).GetRadian<32>();
			}
			break;
		case VectorOrigin::Target:
			{
				AbstractClass* pTgt = pBullet ? pBullet->Target : (pTechno ? pTechno->Target : nullptr);
				TechnoClass* pTT = abstract_cast<TechnoClass*>(pTgt);
				if (pTT && !IsDeadOrInvisible(pTT))
					facingU = (d.OriginIsOnBody.Get() ? pTT->PrimaryFacing.Current() : pTT->TurretFacing()).GetRadian<32>();
				// 目标无朝向（格子）：保持 effectiveFacing（连线）
			}
			break;
		case VectorOrigin::Source:
			{
				TechnoClass* pST = abstract_cast<TechnoClass*>(s._pSource);
				if (pST && !IsDeadOrInvisible(pST))
					facingU = (d.OriginIsOnBody.Get() ? pST->PrimaryFacing.Current() : pST->TurretFacing()).GetRadian<32>();
			}
			break;
		default: // Self：自身朝向即 effectiveFacing
			break;
		}
		double tiltU = originTerrainTilt; // 单位倾斜（AngleRotatedForwards 动态/地形采样）

		// 基础法向量（Init 锁定）→ 笛卡尔；无自定义法线时默认水平圆面（法线朝上）
		double baseTilt = hasNormal ? s._tiltRad : M_PI / 2.0;
		double bx = std::cos(baseTilt) * std::cos(s._facingRad);
		double by = std::cos(baseTilt) * std::sin(s._facingRad);
		double bz = std::sin(baseTilt);

		// 1. 绕 Z 轴转 facingU（单位水平朝向）
		double cz = std::cos(facingU), sz = std::sin(facingU);
		double x1 = bx * cz - by * sz;
		double y1 = bx * sz + by * cz;
		double z1 = bz;

		// 2. 绕单位 L 轴 u=(-sinFU, cosFU, 0) 转 tiltU（Rodrigues）
		double ct = std::cos(tiltU), st = std::sin(tiltU);
		double ux = -sz, uy = cz, uz = 0.0;
		double dot = ux * x1 + uy * y1;      // u·n
		double cx = uy * z1 - uz * y1;       // u×n
		double cy = uz * x1 - ux * z1;
		double cz2 = ux * y1 - uy * x1;
		s._motion.normalX = x1 * ct + cx * st + ux * dot * (1.0 - ct);
		s._motion.normalY = y1 * ct + cy * st + uy * dot * (1.0 - ct);
		s._motion.normalZ = z1 * ct + cz2 * st;

		// 同步倾斜圆面数学输入（最终法向量 → 球坐标）
		double lenXY = std::sqrt(s._motion.normalX * s._motion.normalX + s._motion.normalY * s._motion.normalY);
		effectiveFacing = lenXY > 1e-6 ? std::atan2(s._motion.normalY, s._motion.normalX) : 0.0;
		effectiveTilt = lenXY > 1e-6 ? std::atan2(s._motion.normalZ, lenXY) : (s._motion.normalZ > 0 ? M_PI / 2.0 : -M_PI / 2.0);
	}

	// 3D 法向量旋转覆盖：当 NormalF/L/HAnglePerStep 设定时，无视基座变化
	if (s._motion.normalStepF != 0.0 || s._motion.normalStepL != 0.0 || s._motion.normalStepH != 0.0)
	{
		double lenXY = std::sqrt(s._motion.normalX * s._motion.normalX + s._motion.normalY * s._motion.normalY);
		effectiveFacing = lenXY > 1e-6 ? std::atan2(s._motion.normalY, s._motion.normalX) : 0.0;
		effectiveTilt = lenXY > 1e-6 ? std::atan2(s._motion.normalZ, lenXY) : (s._motion.normalZ > 0 ? M_PI / 2.0 : -M_PI / 2.0);
		mainFacingDir = Radians2Dir(effectiveFacing);
	}

	// ========================================================================
	// Origin 坐标（主 Origin 计算点）
	// ========================================================================
	CoordStruct originPos = currentPos;

	switch (d.Origin.Get())
	{
	case VectorOrigin::Target:
		if (d.OriginNoUpdate.Get())
			originPos = IsEmpty(s._initialOriginPos) ? currentPos : s._initialOriginPos; // 锁定初始目标
		else
		{
			// 允许更新（NoUpdate=no）：引擎链获取；取不到回退锁定坐标
			CoordStruct updated{};
			bool gotUpdate = GetTargetPosFromChain(pBullet, pTechno, updated);
			if (gotUpdate)
			{
				s._initialOriginPos = updated; // 跟随：更新锁定值
				originPos = s._initialOriginPos;
			}
			else
				originPos = IsEmpty(s._initialOriginPos) ? currentPos : s._initialOriginPos; // 回退锁定坐标
		}
		break;
	case VectorOrigin::Launcher:
		if (d.OriginNoUpdate.Get())
			originPos = s._initialOriginPos;
		else
		{
			TrackOriginCoord(s._pLauncher, false, s._initialOriginPos); // 发射者活着：每帧快照；死亡：冻结
			originPos = s._initialOriginPos;
		}
		break;
	case VectorOrigin::Source:
		if (d.OriginNoUpdate.Get())
			originPos = s._initialOriginPos;
		else
		{
			TrackOriginCoord(s._pSource, false, s._initialOriginPos); // 来源活着：每帧快照；死亡：冻结
			originPos = s._initialOriginPos;
		}
		break;
	case VectorOrigin::Self:
		originPos = d.OriginNoUpdate.Get() ? s._initialOriginPos : currentPos;
		break;
	}

	// OriginFLH 偏移：对非 Self 模式生效
	// AllowOriginTilt 时跳过二维 GetFLHAbsoluteCoords，后续用三维旋转处理
	if (!d.AllowOriginTilt.Get())
	{
		if (!IsEmpty(d.OriginFLH.Get()) && d.Origin.Get() != VectorOrigin::Self)
			originPos = GetFLHAbsoluteCoords(originPos, d.OriginFLH.Get(), mainFacingDir);
	}

	// ========================================================================
	// 模式分发
	// ========================================================================

	// 模式 C: Circle（独立圆周，圆心=Origin，三选二参数）
	bool hasCircle = d.CircleRadius.Get() > 0 || d.CircleAnglePerStep.Get() > 0.0
		|| (d.CircleRandomRadiusMax > d.CircleRandomRadiusMin)
		|| (d.CircleRandomAngleMax > d.CircleRandomAngleMin);
	if (hasCircle)
	{
		StepCircle(s, d, pBullet, pTechno, pObject, pSource, currentPos, originPos,
			effectiveFacing, effectiveTilt, originTerrainTilt, hasNormal, duration, out);
		return;
	}

	// 模式 1: MoveTo（纯 FLH 位移 + GrowRate）
	if (!IsEmpty(d.MoveTo.Get()))
	{
		StepMoveTo(s, d, pBullet, pTechno, pObject, currentPos, mainFacingDir, effectiveTilt, out);
		return;
	}

	// 模式 2/5: TargetFLH（ReachTarget / Speed）
	StepTargetFLH(s, d, pBullet, pTechno, pObject, currentPos, originPos, mainFacingDir, out);
}
