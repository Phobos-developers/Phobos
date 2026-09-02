#pragma once
// ============================================================================
// VectorRevibed — 数据层（Kratos VectorDataReVibed 移植到 PhobosAI）
//
// 铁律：INI 标签名、默认值、解析语义与 Kratos 新版（VectorDataReVibed.h）
// 逐字一致，不得借移植之机改动任何标签行为。轨迹算法由逻辑层原样照搬。
//
// PhobosAI 适配点（仅基建，不影响标签语义）：
//   - INIBufferReader -> INI_EX（Valueable 成员 + 手动字符串解析）
//   - IsOnOrigin 默认值按已解析的 Origin 类型推导（与 Kratos 一致）
//   - Enable 推导式原样保留（显式 Vector= 引用挂载时不用它，仅作数据完整性）
// ============================================================================

#include <string>
#include <vector>
#include <sstream>

#include <GeneralStructures.h>
#include <Fundamentals.h>

#include <Utilities/Constructs.h>
#include <Utilities/INIParser.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Stream.h>

// Vector Origin 枚举（自 AttachEffectTypeClass 迁移，保持与 Kratos 语义一致）
enum class VectorOrigin : int
{
	Self = 0,
	Launcher = 1,
	Target = 2,
	Source = 3,
};

// read<VectorOrigin> 特化：必须与本头同文件（VectorRevibedData::Read 是内联函数，
// 任何 include 本头的编译单元实例化 Valueable<VectorOrigin>::Read 时都要看到特化，
// 否则撞 detail::read 的 =delete 主模板（C2280）。放 VectorTypeClass.h 不够——本头
// 被 VectorRevibedState.cpp 等直接 include 而 VectorTypeClass.h 未必可见。
namespace detail
{
	template <>
	inline bool read<VectorOrigin>(VectorOrigin& value, INI_EX& parser, const char* pSection, const char* pKey)
	{
		if (parser.ReadString(pSection, pKey))
		{
			auto str = parser.value();

			if (!_strcmpi(str, "Self"))
				value = VectorOrigin::Self;
			else if (!_strcmpi(str, "Launcher"))
				value = VectorOrigin::Launcher;
			else if (!_strcmpi(str, "Target"))
				value = VectorOrigin::Target;
			else if (!_strcmpi(str, "Source"))
				value = VectorOrigin::Source;
			else
				value = VectorOrigin::Self;

			return true;
		}

		return false;
	}
}

class VectorRevibedData
{
public:
	// ========================================================================
	// 通用设定
	// ========================================================================

	Valueable<int> TimeStep;
	Valueable<int> DisabledFrames;              // 首帧快照后冻结 N 帧，不计入运动时间
	Valueable<bool> SyncFacing;                 // yes=抛射体朝运动方向/单位转动，no=抛射体朝目标
	Valueable<bool> OriginIsOnWorld;            // yes=OriginFLH用世界FLH(朝北)，不使用单位/弹体朝向
	Valueable<bool> OriginIsOnBody;             // yes=单位取车身PrimaryFacing，无视炮塔TurretFacing
	Valueable<VectorOrigin> Origin;
	Valueable<CoordStruct> OriginFLH;
	Valueable<bool> OriginNoUpdate;
	Valueable<bool> Force;                      // yes=SetLocation 硬控，默认所有 Vector 模式 Force
	Valueable<bool> Freeze;
	Valueable<bool> AllowCircleTilt;            // yes=允许圆面使用 NormalVector 或地形倾斜
	bool IsOnOrigin;                            // （INI: Vector.OriginIsOnVectorOrigin）F 轴来源：yes=Origin 单位自身朝向，no=Origin→弹体连线
												// 默认按 Origin 类型推导（Launcher/Self→yes，Target/Source→no），与旧版行为一致
	Valueable<bool> IsNormalOnOrigin;           // 圆面法向量：yes（默认）=每帧跟随 Origin 单位自身朝向转动，no=世界固定

	// ========================================================================
	// NormalVector 圆面法线
	// ========================================================================

	Valueable<CoordStruct> NormalVector;        // 圆面法向量（FLH 坐标系），F/L/H
	Valueable<CoordStruct> NormalRandomF;       // F 分量随机范围 .X=Min .Y=Max
	Valueable<CoordStruct> NormalRandomL;       // L 分量随机范围
	Valueable<CoordStruct> NormalRandomH;       // H 分量随机范围
	Valueable<double> NormalFAnglePerStep;      // 绕 F 轴角速度 (°/step)，常数模式
	Valueable<double> NormalLAnglePerStep;      // 绕 L 轴
	Valueable<double> NormalHAnglePerStep;      // 绕 H 轴
	double NormalFAngleRMin = 0.0, NormalFAngleRMax = 0.0;     // 绕 F 角速度区间1
	double NormalFAngleRMin2 = 0.0, NormalFAngleRMax2 = 0.0;  // 绕 F 角速度区间2
	double NormalLAngleRMin = 0.0, NormalLAngleRMax = 0.0;
	double NormalLAngleRMin2 = 0.0, NormalLAngleRMax2 = 0.0;
	double NormalHAngleRMin = 0.0, NormalHAngleRMax = 0.0;
	double NormalHAngleRMin2 = 0.0, NormalHAngleRMax2 = 0.0;
	Valueable<double> Lissajous;                // 小圆圆周 F 轴偏移角速度（°/step），0=不偏移

	// ========================================================================
	// MoveTo 模式（纯 FLH 位移 + GrowRate）
	// ========================================================================

	Valueable<CoordStruct> MoveTo;
	Valueable<CoordStruct> GrowRate;            // 每帧 FLH 增量（呼吸/螺旋/振幅）
	Valueable<double> AnglePerStep;             // MoveTo 模式角度自增（°/step）

	// ========================================================================
	// Circle 模式（圆周，独立于 MoveTo）
	// 三选二：Radius / Speed / AnglePerStep，未设的一项自动推算
	// 圆心 = Origin（同 Origin 标签，动态刷新除非 NoUpdate）
	// ========================================================================

	Valueable<int> CircleRadius;                // 圆半径（lepton），-1=自动取当前XY距离
	Valueable<int> CircleSpeed;                 // 线速度（lepton/step沿圆周），0=不由它推算
	Valueable<int> CircleSpeedAcceleration;     // 线速度每步加速度
	Valueable<int> CircleMaxSpeed;              // 线速度上限，0=不限
	Valueable<int> CircleMinSpeed;              // 线速度下限，0=不限
	Valueable<double> CircleAnglePerStep;       // 角速度（°/step），0=不由它推算
	Valueable<double> CircleAngleAcceleration;  // 角速度每步加速度
	Valueable<double> CircleMaxAngle;           // 角速度上限，0=不限
	Valueable<double> CircleMinAngle;           // 角速度下限，0=不限
	Valueable<CoordStruct> CircleOrigin;        // 圆心偏移（默认世界坐标，AllowOriginTilt=yes 时 FLH 旋转）
	Valueable<bool> AllowOriginTilt;            // yes=圆心偏移跟随转轴倾斜
	int CircleRandomRadiusMin = 0;              // 初始半径随机下限
	int CircleRandomRadiusMax = 0;              // 初始半径随机上限
	double CircleRandomAngleMin = 0.0;          // 初始角速度随机下限
	double CircleRandomAngleMax = 0.0;          // 初始角速度随机上限
	double CircleRandomAngleMin2 = 0.0;         // 第二区间下限（4 值模式）
	double CircleRandomAngleMax2 = 0.0;         // 第二区间上限（4 值模式）
	Valueable<int> CircleRadiusGrow;            // 半径每步增长量（lepton/step），正=外扩，负=内缩
	Valueable<int> CircleMaxRadius;             // 半径上限，0=不限
	Valueable<int> CircleMinRadius;             // 半径下限，0=不限
	Valueable<bool> CircleEndOnMaxRadius;       // 半径达到上限时结束 AE
	Valueable<bool> CircleEndOnMinRadius;       // 半径达到下限时结束 AE

	// ========================================================================
	// 圆心运动（Vector.Origin.* 系列，Circle 模式专用）
	// ========================================================================

	// MoveTo 模式
	Valueable<CoordStruct> OriginMoveTo;        // 圆心 FLH 位移（lepton/step）
	Valueable<CoordStruct> OriginGrowRate;      // 每步增量
	Valueable<double> OriginAnglePerStep;       // 自旋角度（°/step）
	// Speed / ReachTarget 模式
	Valueable<CoordStruct> OriginTargetFLH;     // 追踪目标 FLH
	Valueable<int> OriginLinearSpeed;           // 初始速度，-1=读取单位Speed
	Valueable<int> OriginAcceleration;          // 加速度（lepton/step²）
	Valueable<int> OriginMaxSpeed;              // 最大速度，-1=不限
	Valueable<int> OriginMinSpeed;              // 最小速度，-1=不限
	Valueable<bool> OriginReachTarget;          // 到达模式
	Valueable<bool> OriginSpeedEndOnReach;      // Speed模式抵达目标即结束AE
	Valueable<int> OriginArcHeight;             // 到达模式弧高
	Valueable<double> OriginArcPeakPercent;     // 弧高点百分比（0-100），默认50=中点
	Valueable<Point2D> OriginArcPeakRandomPercent; // 随机弧高点百分比
	int OriginArcRandomHeightMin = 0;           // 随机弧高下限
	int OriginArcRandomHeightMax = 0;           // 随机弧高上限
	Valueable<double> OriginArcRotation;        // 弧面旋转角（°），0=朝上
	double OriginArcRandomRotationMin = 0.0;    // 随机旋转下限
	double OriginArcRandomRotationMax = 0.0;    // 随机旋转上限
	int OriginTargetOffsetFMin = 0, OriginTargetOffsetFMax = 0;
	int OriginTargetOffsetLMin = 0, OriginTargetOffsetLMax = 0;
	int OriginTargetOffsetHMin = 0, OriginTargetOffsetHMax = 0;
	// Circle 模式
	Valueable<int> OriginCircleRadius;
	Valueable<int> OriginCircleSpeed;
	Valueable<double> OriginCircleAnglePerStep;
	int OriginCircleRandomRadiusMin = 0, OriginCircleRandomRadiusMax = 0;
	double OriginCircleRandomAngleMin = 0, OriginCircleRandomAngleMax = 0;
	Valueable<int> OriginCircleRadiusGrow;
	Valueable<int> OriginCircleMaxRadius;
	Valueable<int> OriginCircleMinRadius;
	Valueable<bool> OriginCircleEndOnMaxRadius;
	Valueable<bool> OriginCircleEndOnMinRadius;
	// 法线
	Valueable<CoordStruct> OriginNormalVector;
	Valueable<CoordStruct> OriginNormalRandomF;
	Valueable<CoordStruct> OriginNormalRandomL;
	Valueable<CoordStruct> OriginNormalRandomH;
	Valueable<double> OriginNormalFAnglePerStep;
	Valueable<double> OriginNormalLAnglePerStep;
	Valueable<double> OriginNormalHAnglePerStep;
	double OriginNormalFAngleRMin = 0, OriginNormalFAngleRMax = 0, OriginNormalFAngleRMin2 = 0, OriginNormalFAngleRMax2 = 0;
	double OriginNormalLAngleRMin = 0, OriginNormalLAngleRMax = 0, OriginNormalLAngleRMin2 = 0, OriginNormalLAngleRMax2 = 0;
	double OriginNormalHAngleRMin = 0, OriginNormalHAngleRMax = 0, OriginNormalHAngleRMin2 = 0, OriginNormalHAngleRMax2 = 0;
	// 圆心通用
	Valueable<bool> OriginAllowCircleTilt;      // yes=大圆面跟随目标倾斜（Origin=Target 时有效）
	Valueable<bool> OriginIsNormalOnOrigin;     // 大圆法向量：yes（默认）=每帧跟随 OriginOrigin 单位自身朝向转动，no=世界固定
	Valueable<CoordStruct> OriginCircleOffset;  // 圆心原点偏移（世界坐标）
	Valueable<bool> OriginAllowOriginTilt;
	Valueable<bool> OriginOriginNoUpdate;       // yes=圆心基座冻结在初始位置，不随目标移动
	Valueable<double> OriginLissajous;          // 大圆圆周 F 轴偏移角速度（°/step），0=不偏移
	Valueable<VectorOrigin> OriginOrigin;       // 圆心运动参考系
	Valueable<CoordStruct> OriginOriginFLH;     // OriginOrigin=FLH 时的 FLH 偏移

	// ========================================================================
	// Speed 模式（直线追踪 + 加速度）
	// ========================================================================

	Valueable<CoordStruct> TargetFLH;
	int TargetOffsetFMin = 0;
	int TargetOffsetFMax = 0;
	int TargetOffsetLMin = 0;
	int TargetOffsetLMax = 0;
	int TargetOffsetHMin = 0;
	int TargetOffsetHMax = 0;
	// 四参数版（TargetOffsetFRanges）：区间1复用上方 Min/Max，区间2独立存储，双区间50%取一
	int TargetOffsetFMin2 = 0;
	int TargetOffsetFMax2 = 0;
	int TargetOffsetLMin2 = 0;
	int TargetOffsetLMax2 = 0;
	int TargetOffsetHMin2 = 0;
	int TargetOffsetHMax2 = 0;
	// 半径模式（TargetOffsetRadius）：与 F/L/H 互斥，全向随机落点
	int TargetOffsetRadiusMin = 0;
	int TargetOffsetRadiusMax = 0;
	int TargetOffsetRadiusMin2 = 0;  // 四参数版（TargetOffsetRadiusRanges）区间2，区间1复用 Min/Max
	int TargetOffsetRadiusMax2 = 0;
	Valueable<bool> TargetOffsetSphere;         // yes=球面全向（含H），no=XY圆环+H用TargetOffsetH
	Valueable<CoordStruct> TargetOffsetNormal;  // 圆环法向量（FLH），非空时 TargetOffsetSphere=no 的落点在倾斜圆面上（法向量定义圆面）
	// 角度限制（TargetOffsetAngles，仅圆环模式）：双区间，0度=目标点指向抛射体（近交点）
	int TargetOffsetAngleMin = 0;
	int TargetOffsetAngleMax = 0;
	int TargetOffsetAngleMin2 = 0;
	int TargetOffsetAngleMax2 = 0;

	Valueable<int> LinearSpeed;                 // -1 = 读取单位 Speed
	int RandomSpeedMin = 0;                     // Speed 模式随机速度下限
	int RandomSpeedMax = 0;                     // Speed 模式随机速度上限
	Valueable<int> MaxSpeed;                    // -1 = 不限
	Valueable<int> MinSpeed;                    // -1 = 不限
	Valueable<int> Acceleration;                // 每帧速度增量
	Valueable<bool> SpeedEndOnReach;            // 抵达目标坐标点即强制结束AE（修复飞越后抽搐）

	// ========================================================================
	// ReachTarget 模式（剩余帧数强制到达）
	// ========================================================================

	Valueable<bool> ReachTarget;                // 与 TargetFLH 配合使用
	Valueable<int> ReachTargetEarlyEnd;         // 提前结束 AE 的帧数，0=禁用，>0 时提前 N 帧交还引擎
	Valueable<int> ArcHeight;                   // ReachTarget 弧高（lepton），0=直线，正=上凸
	Valueable<double> ArcPeakPercent;           // 弧高点所在 Duration 百分比（0-100），默认50=中点
	Valueable<Point2D> ArcPeakRandomPercent;    // 随机弧高百分比范围 (Min, Max)
	int ArcRandomHeightMin = 0;                 // 随机弧高下限
	int ArcRandomHeightMax = 0;                 // 随机弧高上限
	Valueable<double> ArcRotation;              // 弧面旋转角（°），0=默认朝上，顺时针
	double ArcRandomRotationMin = 0.0;          // 随机旋转下限
	double ArcRandomRotationMax = 0.0;          // 随机旋转上限

	// ========================================================================
	// 内部
	// ========================================================================

	Valueable<bool> Enable;                     // 推导式启用（与 Kratos 一致，显式引用挂载时不用）

	VectorRevibedData()
		: TimeStep { 1 }
		, DisabledFrames { 0 }
		, SyncFacing { false }
		, OriginIsOnWorld { false }
		, OriginIsOnBody { false }
		, Origin { VectorOrigin::Self }
		, OriginFLH { CoordStruct::Empty }
		, OriginNoUpdate { false }
		, Force { true }
		, Freeze { false }
		, AllowCircleTilt { true }
		, IsOnOrigin { false }
		, IsNormalOnOrigin { true }
		, NormalVector { CoordStruct::Empty }
		, NormalRandomF { CoordStruct::Empty }
		, NormalRandomL { CoordStruct::Empty }
		, NormalRandomH { CoordStruct::Empty }
		, NormalFAnglePerStep { 0.0 }
		, NormalLAnglePerStep { 0.0 }
		, NormalHAnglePerStep { 0.0 }
		, Lissajous { 0.0 }
		, MoveTo { CoordStruct::Empty }
		, GrowRate { CoordStruct::Empty }
		, AnglePerStep { 0.0 }
		, CircleRadius { -1 }
		, CircleSpeed { 0 }
		, CircleSpeedAcceleration { 0 }
		, CircleMaxSpeed { 0 }
		, CircleMinSpeed { 0 }
		, CircleAnglePerStep { 0.0 }
		, CircleAngleAcceleration { 0.0 }
		, CircleMaxAngle { 0.0 }
		, CircleMinAngle { 0.0 }
		, CircleOrigin { CoordStruct::Empty }
		, AllowOriginTilt { true }
		, CircleRadiusGrow { 0 }
		, CircleMaxRadius { 0 }
		, CircleMinRadius { 0 }
		, CircleEndOnMaxRadius { false }
		, CircleEndOnMinRadius { false }
		, OriginMoveTo { CoordStruct::Empty }
		, OriginGrowRate { CoordStruct::Empty }
		, OriginAnglePerStep { 0.0 }
		, OriginTargetFLH { CoordStruct::Empty }
		, OriginLinearSpeed { -1 }
		, OriginAcceleration { 0 }
		, OriginMaxSpeed { -1 }
		, OriginMinSpeed { -1 }
		, OriginReachTarget { false }
		, OriginSpeedEndOnReach { true }
		, OriginArcHeight { 0 }
		, OriginArcPeakPercent { 50.0 }
		, OriginArcPeakRandomPercent { { 0, 0 } }
		, OriginArcRotation { 0.0 }
		, OriginCircleRadius { -1 }
		, OriginCircleSpeed { 0 }
		, OriginCircleAnglePerStep { 0.0 }
		, OriginCircleRadiusGrow { 0 }
		, OriginCircleMaxRadius { 0 }
		, OriginCircleMinRadius { 0 }
		, OriginCircleEndOnMaxRadius { false }
		, OriginCircleEndOnMinRadius { false }
		, OriginNormalVector { CoordStruct::Empty }
		, OriginNormalRandomF { CoordStruct::Empty }
		, OriginNormalRandomL { CoordStruct::Empty }
		, OriginNormalRandomH { CoordStruct::Empty }
		, OriginNormalFAnglePerStep { 0.0 }
		, OriginNormalLAnglePerStep { 0.0 }
		, OriginNormalHAnglePerStep { 0.0 }
		, OriginAllowCircleTilt { true }
		, OriginIsNormalOnOrigin { true }
		, OriginCircleOffset { CoordStruct::Empty }
		, OriginAllowOriginTilt { true }
		, OriginOriginNoUpdate { false }
		, OriginLissajous { 0.0 }
		, OriginOrigin { VectorOrigin::Self }
		, OriginOriginFLH { CoordStruct::Empty }
		, TargetFLH { CoordStruct::Empty }
		, TargetOffsetSphere { false }
		, TargetOffsetNormal { CoordStruct::Empty }
		, LinearSpeed { -1 }
		, MaxSpeed { -1 }
		, MinSpeed { -1 }
		, Acceleration { 0 }
		, SpeedEndOnReach { true }
		, ReachTarget { false }
		, ReachTargetEarlyEnd { 0 }
		, ArcHeight { 0 }
		, ArcPeakPercent { 50.0 }
		, ArcPeakRandomPercent { { 0, 0 } }
		, ArcRotation { 0.0 }
		, Enable { false }
	{ }

	void Read(INI_EX& reader, const char* pSection)
	{
		const std::string title = "Vector.";

		// --- 通用 ---
		this->TimeStep.Read(reader, pSection, (title + "TimeStep").c_str());
		if (this->TimeStep.Get() < 1)
			this->TimeStep = 1;
		this->DisabledFrames.Read(reader, pSection, (title + "DisabledFrames").c_str());
		this->SyncFacing.Read(reader, pSection, (title + "SyncFacing").c_str());
		this->OriginIsOnWorld.Read(reader, pSection, (title + "OriginIsOnWorld").c_str());
		this->OriginIsOnBody.Read(reader, pSection, (title + "OriginIsOnBody").c_str());
		this->Origin.Read(reader, pSection, (title + "Origin").c_str());
		this->OriginFLH.Read(reader, pSection, (title + "OriginFLH").c_str());
		this->OriginNoUpdate.Read(reader, pSection, (title + "OriginNoUpdate").c_str());
		this->Force.Read(reader, pSection, (title + "Force").c_str());
		this->Freeze.Read(reader, pSection, (title + "Freeze").c_str());
		this->AllowCircleTilt.Read(reader, pSection, (title + "AllowCircleTilt").c_str());
		// 默认按 Origin 类型推导旧版行为：Launcher/Self=单位自身朝向(yes)，Target/Source=连线(no)
		this->IsOnOrigin = (this->Origin.Get() == VectorOrigin::Launcher || this->Origin.Get() == VectorOrigin::Self);
		reader.ReadBool(pSection, (title + "OriginIsOnVectorOrigin").c_str(), &this->IsOnOrigin);
		this->IsNormalOnOrigin.Read(reader, pSection, (title + "IsNormalOnOrigin").c_str());
		this->NormalVector.Read(reader, pSection, (title + "NormalVector").c_str());
		this->NormalRandomF.Read(reader, pSection, (title + "NormalRandomF").c_str());
		this->NormalRandomL.Read(reader, pSection, (title + "NormalRandomL").c_str());
		this->NormalRandomH.Read(reader, pSection, (title + "NormalRandomH").c_str());
		this->NormalFAnglePerStep.Read(reader, pSection, (title + "NormalFAnglePerStep").c_str());
		this->NormalLAnglePerStep.Read(reader, pSection, (title + "NormalLAnglePerStep").c_str());
		this->NormalHAnglePerStep.Read(reader, pSection, (title + "NormalHAnglePerStep").c_str());
		Parse4Double(reader, pSection, title + "NormalFAngleRanges", NormalFAngleRMin, NormalFAngleRMax, NormalFAngleRMin2, NormalFAngleRMax2);
		Parse4Double(reader, pSection, title + "NormalLAngleRanges", NormalLAngleRMin, NormalLAngleRMax, NormalLAngleRMin2, NormalLAngleRMax2);
		Parse4Double(reader, pSection, title + "NormalHAngleRanges", NormalHAngleRMin, NormalHAngleRMax, NormalHAngleRMin2, NormalHAngleRMax2);
		this->Lissajous.Read(reader, pSection, (title + "Lissajous").c_str());

		// --- MoveTo ---
		this->MoveTo.Read(reader, pSection, (title + "MoveTo").c_str());
		this->GrowRate.Read(reader, pSection, (title + "GrowRate").c_str());
		this->AnglePerStep.Read(reader, pSection, (title + "AnglePerStep").c_str());

		// --- Circle ---
		this->CircleRadius.Read(reader, pSection, (title + "CircleRadius").c_str());
		this->CircleSpeed.Read(reader, pSection, (title + "CircleSpeed").c_str());
		this->CircleSpeedAcceleration.Read(reader, pSection, (title + "CircleSpeedAcceleration").c_str());
		this->CircleMaxSpeed.Read(reader, pSection, (title + "CircleMaxSpeed").c_str());
		this->CircleMinSpeed.Read(reader, pSection, (title + "CircleMinSpeed").c_str());
		this->CircleAnglePerStep.Read(reader, pSection, (title + "CircleAnglePerStep").c_str());
		this->CircleAngleAcceleration.Read(reader, pSection, (title + "CircleAngleAcceleration").c_str());
		this->CircleMaxAngle.Read(reader, pSection, (title + "CircleMaxAngle").c_str());
		this->CircleMinAngle.Read(reader, pSection, (title + "CircleMinAngle").c_str());
		this->CircleOrigin.Read(reader, pSection, (title + "CircleOrigin").c_str());
		this->AllowOriginTilt.Read(reader, pSection, (title + "AllowOriginTilt").c_str());
		ParseMinMaxInt(reader, pSection, title + "CircleRandomRadius", CircleRandomRadiusMin, CircleRandomRadiusMax);
		{
			// 单标签双值（Kratos 原样：仅含逗号时解析，单值不生效）
			std::string str;
			if (ReadStringValue(reader, pSection, title + "CircleRandomAngle", str))
			{
				auto comma = str.find(',');
				if (comma != std::string::npos)
				{
					CircleRandomAngleMin = std::stod(str.substr(0, comma));
					CircleRandomAngleMax = std::stod(str.substr(comma + 1));
				}
			}
		}
		{
			// 四参数版：min1,max1,min2,max2
			double angles[4];
			if (ReadDoubles(reader, pSection, title + "CircleRandomAngleRanges", angles, 4))
			{
				CircleRandomAngleMin = angles[0];
				CircleRandomAngleMax = angles[1];
				CircleRandomAngleMin2 = angles[2];
				CircleRandomAngleMax2 = angles[3];
			}
		}
		this->CircleRadiusGrow.Read(reader, pSection, (title + "CircleRadiusGrow").c_str());
		this->CircleMaxRadius.Read(reader, pSection, (title + "CircleMaxRadius").c_str());
		this->CircleMinRadius.Read(reader, pSection, (title + "CircleMinRadius").c_str());
		this->CircleEndOnMaxRadius.Read(reader, pSection, (title + "CircleEndOnMaxRadius").c_str());
		this->CircleEndOnMinRadius.Read(reader, pSection, (title + "CircleEndOnMinRadius").c_str());

		// --- Origin ---
		this->OriginMoveTo.Read(reader, pSection, (title + "Origin.MoveTo").c_str());
		this->OriginGrowRate.Read(reader, pSection, (title + "Origin.GrowRate").c_str());
		this->OriginAnglePerStep.Read(reader, pSection, (title + "Origin.AnglePerStep").c_str());
		this->OriginTargetFLH.Read(reader, pSection, (title + "Origin.TargetFLH").c_str());
		this->OriginLinearSpeed.Read(reader, pSection, (title + "Origin.LinearSpeed").c_str());
		this->OriginAcceleration.Read(reader, pSection, (title + "Origin.Acceleration").c_str());
		this->OriginMaxSpeed.Read(reader, pSection, (title + "Origin.MaxSpeed").c_str());
		this->OriginMinSpeed.Read(reader, pSection, (title + "Origin.MinSpeed").c_str());
		this->OriginReachTarget.Read(reader, pSection, (title + "Origin.ReachTarget").c_str());
		this->OriginSpeedEndOnReach.Read(reader, pSection, (title + "Origin.SpeedEndOnReach").c_str());
		this->OriginArcHeight.Read(reader, pSection, (title + "Origin.ArcHeight").c_str());
		this->OriginArcPeakPercent.Read(reader, pSection, (title + "Origin.ArcPeakPercent").c_str());
		this->OriginArcPeakRandomPercent.Read(reader, pSection, (title + "Origin.RandomArcPeakPercent").c_str());
		ParseMinMaxInt(reader, pSection, title + "Origin.RandomArcHeight", OriginArcRandomHeightMin, OriginArcRandomHeightMax);
		this->OriginArcRotation.Read(reader, pSection, (title + "Origin.ArcRotation").c_str());
		ParseMinMaxDouble(reader, pSection, title + "Origin.RandomArcRotation", OriginArcRandomRotationMin, OriginArcRandomRotationMax);
		ParseMinMaxInt(reader, pSection, title + "Origin.TargetOffsetF", OriginTargetOffsetFMin, OriginTargetOffsetFMax);
		ParseMinMaxInt(reader, pSection, title + "Origin.TargetOffsetL", OriginTargetOffsetLMin, OriginTargetOffsetLMax);
		ParseMinMaxInt(reader, pSection, title + "Origin.TargetOffsetH", OriginTargetOffsetHMin, OriginTargetOffsetHMax);
		this->OriginCircleRadius.Read(reader, pSection, (title + "Origin.CircleRadius").c_str());
		this->OriginCircleSpeed.Read(reader, pSection, (title + "Origin.CircleSpeed").c_str());
		this->OriginCircleAnglePerStep.Read(reader, pSection, (title + "Origin.CircleAnglePerStep").c_str());
		ParseMinMaxInt(reader, pSection, title + "Origin.CircleRandomRadius", OriginCircleRandomRadiusMin, OriginCircleRandomRadiusMax);
		ParseMinMaxDouble(reader, pSection, title + "Origin.CircleRandomAngle", OriginCircleRandomAngleMin, OriginCircleRandomAngleMax);
		this->OriginCircleRadiusGrow.Read(reader, pSection, (title + "Origin.CircleRadiusGrow").c_str());
		this->OriginCircleMaxRadius.Read(reader, pSection, (title + "Origin.CircleMaxRadius").c_str());
		this->OriginCircleMinRadius.Read(reader, pSection, (title + "Origin.CircleMinRadius").c_str());
		this->OriginCircleEndOnMaxRadius.Read(reader, pSection, (title + "Origin.CircleEndOnMaxRadius").c_str());
		this->OriginCircleEndOnMinRadius.Read(reader, pSection, (title + "Origin.CircleEndOnMinRadius").c_str());
		this->OriginNormalVector.Read(reader, pSection, (title + "Origin.NormalVector").c_str());
		this->OriginNormalRandomF.Read(reader, pSection, (title + "Origin.NormalRandomF").c_str());
		this->OriginNormalRandomL.Read(reader, pSection, (title + "Origin.NormalRandomL").c_str());
		this->OriginNormalRandomH.Read(reader, pSection, (title + "Origin.NormalRandomH").c_str());
		this->OriginNormalFAnglePerStep.Read(reader, pSection, (title + "Origin.NormalFAnglePerStep").c_str());
		this->OriginNormalLAnglePerStep.Read(reader, pSection, (title + "Origin.NormalLAnglePerStep").c_str());
		this->OriginNormalHAnglePerStep.Read(reader, pSection, (title + "Origin.NormalHAnglePerStep").c_str());
		Parse4Double(reader, pSection, title + "Origin.NormalFAngleRanges", OriginNormalFAngleRMin, OriginNormalFAngleRMax, OriginNormalFAngleRMin2, OriginNormalFAngleRMax2);
		Parse4Double(reader, pSection, title + "Origin.NormalLAngleRanges", OriginNormalLAngleRMin, OriginNormalLAngleRMax, OriginNormalLAngleRMin2, OriginNormalLAngleRMax2);
		Parse4Double(reader, pSection, title + "Origin.NormalHAngleRanges", OriginNormalHAngleRMin, OriginNormalHAngleRMax, OriginNormalHAngleRMin2, OriginNormalHAngleRMax2);
		this->OriginAllowCircleTilt.Read(reader, pSection, (title + "Origin.AllowCircleTilt").c_str());
		this->OriginIsNormalOnOrigin.Read(reader, pSection, (title + "Origin.IsNormalOnOrigin").c_str());
		this->OriginCircleOffset.Read(reader, pSection, (title + "Origin.CircleOrigin").c_str());
		this->OriginAllowOriginTilt.Read(reader, pSection, (title + "Origin.AllowOriginTilt").c_str());
		this->OriginOriginNoUpdate.Read(reader, pSection, (title + "Origin.OriginNoUpdate").c_str());
		this->OriginLissajous.Read(reader, pSection, (title + "Origin.Lissajous").c_str());
		this->OriginOrigin.Read(reader, pSection, (title + "Origin.Origin").c_str());
		this->OriginOriginFLH.Read(reader, pSection, (title + "Origin.OriginFLH").c_str());

		// --- Speed / ReachTarget ---
		this->TargetFLH.Read(reader, pSection, (title + "TargetFLH").c_str());
		ParseMinMaxInt(reader, pSection, title + "TargetOffsetF", TargetOffsetFMin, TargetOffsetFMax);
		ParseMinMaxInt(reader, pSection, title + "TargetOffsetL", TargetOffsetLMin, TargetOffsetLMax);
		ParseMinMaxInt(reader, pSection, title + "TargetOffsetH", TargetOffsetHMin, TargetOffsetHMax);
		{
			// 四参数版：min1,max1,min2,max2。前两位覆盖区间1（Min/Max），后两位写区间2（Min2/Max2）
			// 双区间全无效时保留两参数值（回退语义）
			Parse4Int(reader, pSection, title + "TargetOffsetFRanges", TargetOffsetFMin, TargetOffsetFMax, TargetOffsetFMin2, TargetOffsetFMax2);
			Parse4Int(reader, pSection, title + "TargetOffsetLRanges", TargetOffsetLMin, TargetOffsetLMax, TargetOffsetLMin2, TargetOffsetLMax2);
			Parse4Int(reader, pSection, title + "TargetOffsetHRanges", TargetOffsetHMin, TargetOffsetHMax, TargetOffsetHMin2, TargetOffsetHMax2);
			Parse4Int(reader, pSection, title + "TargetOffsetAngles", TargetOffsetAngleMin, TargetOffsetAngleMax, TargetOffsetAngleMin2, TargetOffsetAngleMax2);
			Parse4Int(reader, pSection, title + "TargetOffsetRadiusRanges", TargetOffsetRadiusMin, TargetOffsetRadiusMax, TargetOffsetRadiusMin2, TargetOffsetRadiusMax2);
		}
		ParseMinMaxInt(reader, pSection, title + "TargetOffsetRadius", TargetOffsetRadiusMin, TargetOffsetRadiusMax);
		this->TargetOffsetSphere.Read(reader, pSection, (title + "TargetOffsetSphere").c_str());
		this->TargetOffsetNormal.Read(reader, pSection, (title + "TargetOffsetNormal").c_str());
		this->ReachTarget.Read(reader, pSection, (title + "ReachTarget").c_str());
		this->ReachTargetEarlyEnd.Read(reader, pSection, (title + "ReachTargetEarlyEnd").c_str());
		this->ArcHeight.Read(reader, pSection, (title + "ArcHeight").c_str());
		this->ArcPeakPercent.Read(reader, pSection, (title + "ArcPeakPercent").c_str());
		this->ArcPeakRandomPercent.Read(reader, pSection, (title + "RandomArcPeakPercent").c_str());
		ParseMinMaxInt(reader, pSection, title + "RandomArcHeight", ArcRandomHeightMin, ArcRandomHeightMax);
		this->ArcRotation.Read(reader, pSection, (title + "ArcRotation").c_str());
		ParseMinMaxDouble(reader, pSection, title + "RandomArcRotation", ArcRandomRotationMin, ArcRandomRotationMax);

		// --- 速度 ---
		this->LinearSpeed.Read(reader, pSection, (title + "LinearSpeed").c_str());
		ParseMinMaxInt(reader, pSection, title + "RandomSpeed", RandomSpeedMin, RandomSpeedMax);
		this->MaxSpeed.Read(reader, pSection, (title + "MaxSpeed").c_str());
		this->MinSpeed.Read(reader, pSection, (title + "MinSpeed").c_str());
		this->Acceleration.Read(reader, pSection, (title + "Acceleration").c_str());
		this->SpeedEndOnReach.Read(reader, pSection, (title + "SpeedEndOnReach").c_str());

		// --- 启用推导（与 Kratos 逐字一致；PhobosAI 的 Vector3D 无 IsEmpty，用 == Empty 等价） ---
		this->Enable =
			!(this->MoveTo.Get() == CoordStruct::Empty)
			|| this->Freeze.Get()
			|| this->ReachTarget.Get()
			|| (this->LinearSpeed.Get() >= 0)
			|| (this->CircleRadius.Get() > 0)
			|| (this->CircleSpeed.Get() != 0)
			|| (this->CircleAnglePerStep.Get() > 0.0)
			|| (this->CircleRandomRadiusMax > this->CircleRandomRadiusMin)
			|| (this->CircleRandomAngleMax > this->CircleRandomAngleMin)
			|| (this->CircleRandomAngleMax2 > this->CircleRandomAngleMin2);
	}

#pragma region save/load
	// Phobos 存档包装（VectorTypeClass::Serialize 调 .Process(this->Data) 需要）
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) { return this->Serialize(Stm); }
	bool Save(PhobosStreamWriter& Stm) const { return const_cast<VectorRevibedData*>(this)->Serialize(Stm); }

	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->TimeStep)
			.Process(this->DisabledFrames)
			.Process(this->SyncFacing)
			.Process(this->OriginIsOnWorld)
			.Process(this->OriginIsOnBody)
			.Process(this->Origin)
			.Process(this->OriginFLH)
			.Process(this->OriginNoUpdate)
			.Process(this->Force)
			.Process(this->Freeze)
			.Process(this->AllowCircleTilt)
			.Process(this->IsOnOrigin)
			.Process(this->IsNormalOnOrigin)
			.Process(this->NormalVector)
			.Process(this->NormalRandomF)
			.Process(this->NormalRandomL)
			.Process(this->NormalRandomH)
			.Process(this->NormalFAnglePerStep)
			.Process(this->NormalLAnglePerStep)
			.Process(this->NormalHAnglePerStep)
			.Process(this->NormalFAngleRMin)
			.Process(this->NormalFAngleRMax)
			.Process(this->NormalFAngleRMin2)
			.Process(this->NormalFAngleRMax2)
			.Process(this->NormalLAngleRMin)
			.Process(this->NormalLAngleRMax)
			.Process(this->NormalLAngleRMin2)
			.Process(this->NormalLAngleRMax2)
			.Process(this->NormalHAngleRMin)
			.Process(this->NormalHAngleRMax)
			.Process(this->NormalHAngleRMin2)
			.Process(this->NormalHAngleRMax2)
			.Process(this->Lissajous)
			.Process(this->MoveTo)
			.Process(this->GrowRate)
			.Process(this->AnglePerStep)
			.Process(this->CircleRadius)
			.Process(this->CircleSpeed)
			.Process(this->CircleSpeedAcceleration)
			.Process(this->CircleMaxSpeed)
			.Process(this->CircleMinSpeed)
			.Process(this->CircleAnglePerStep)
			.Process(this->CircleAngleAcceleration)
			.Process(this->CircleMaxAngle)
			.Process(this->CircleMinAngle)
			.Process(this->CircleOrigin)
			.Process(this->AllowOriginTilt)
			.Process(this->CircleRandomRadiusMin)
			.Process(this->CircleRandomRadiusMax)
			.Process(this->CircleRandomAngleMin)
			.Process(this->CircleRandomAngleMax)
			.Process(this->CircleRandomAngleMin2)
			.Process(this->CircleRandomAngleMax2)
			.Process(this->CircleRadiusGrow)
			.Process(this->CircleMaxRadius)
			.Process(this->CircleMinRadius)
			.Process(this->CircleEndOnMaxRadius)
			.Process(this->CircleEndOnMinRadius)
			.Process(this->OriginMoveTo)
			.Process(this->OriginGrowRate)
			.Process(this->OriginAnglePerStep)
			.Process(this->OriginTargetFLH)
			.Process(this->OriginLinearSpeed)
			.Process(this->OriginReachTarget)
			.Process(this->OriginSpeedEndOnReach)
			.Process(this->OriginArcHeight)
			.Process(this->OriginArcPeakPercent)
			.Process(this->OriginArcPeakRandomPercent)
			.Process(this->OriginArcRandomHeightMin)
			.Process(this->OriginArcRandomHeightMax)
			.Process(this->OriginArcRotation)
			.Process(this->OriginArcRandomRotationMin)
			.Process(this->OriginArcRandomRotationMax)
			.Process(this->OriginTargetOffsetFMin)
			.Process(this->OriginTargetOffsetFMax)
			.Process(this->OriginTargetOffsetLMin)
			.Process(this->OriginTargetOffsetLMax)
			.Process(this->OriginTargetOffsetHMin)
			.Process(this->OriginTargetOffsetHMax)
			.Process(this->OriginCircleRadius)
			.Process(this->OriginCircleSpeed)
			.Process(this->OriginCircleAnglePerStep)
			.Process(this->OriginCircleRandomRadiusMin)
			.Process(this->OriginCircleRandomRadiusMax)
			.Process(this->OriginCircleRandomAngleMin)
			.Process(this->OriginCircleRandomAngleMax)
			.Process(this->OriginCircleRadiusGrow)
			.Process(this->OriginCircleMaxRadius)
			.Process(this->OriginCircleMinRadius)
			.Process(this->OriginCircleEndOnMaxRadius)
			.Process(this->OriginCircleEndOnMinRadius)
			.Process(this->OriginNormalVector)
			.Process(this->OriginNormalRandomF)
			.Process(this->OriginNormalRandomL)
			.Process(this->OriginNormalRandomH)
			.Process(this->OriginNormalFAnglePerStep)
			.Process(this->OriginNormalLAnglePerStep)
			.Process(this->OriginNormalHAnglePerStep)
			.Process(this->OriginNormalFAngleRMin)
			.Process(this->OriginNormalFAngleRMax)
			.Process(this->OriginNormalFAngleRMin2)
			.Process(this->OriginNormalFAngleRMax2)
			.Process(this->OriginNormalLAngleRMin)
			.Process(this->OriginNormalLAngleRMax)
			.Process(this->OriginNormalLAngleRMin2)
			.Process(this->OriginNormalLAngleRMax2)
			.Process(this->OriginNormalHAngleRMin)
			.Process(this->OriginNormalHAngleRMax)
			.Process(this->OriginNormalHAngleRMin2)
			.Process(this->OriginNormalHAngleRMax2)
			.Process(this->OriginAllowCircleTilt)
			.Process(this->OriginIsNormalOnOrigin)
			.Process(this->OriginCircleOffset)
			.Process(this->OriginAllowOriginTilt)
			.Process(this->OriginOriginNoUpdate)
			.Process(this->OriginLissajous)
			.Process(this->OriginOrigin)
			.Process(this->OriginOriginFLH)
			.Process(this->TargetFLH)
			.Process(this->TargetOffsetFMin)
			.Process(this->TargetOffsetFMax)
			.Process(this->TargetOffsetLMin)
			.Process(this->TargetOffsetLMax)
			.Process(this->TargetOffsetHMin)
			.Process(this->TargetOffsetHMax)
			.Process(this->TargetOffsetFMin2)
			.Process(this->TargetOffsetFMax2)
			.Process(this->TargetOffsetLMin2)
			.Process(this->TargetOffsetLMax2)
			.Process(this->TargetOffsetHMin2)
			.Process(this->TargetOffsetHMax2)
			.Process(this->TargetOffsetRadiusMin)
			.Process(this->TargetOffsetRadiusMax)
			.Process(this->TargetOffsetRadiusMin2)
			.Process(this->TargetOffsetRadiusMax2)
			.Process(this->TargetOffsetSphere)
			.Process(this->TargetOffsetNormal)
			.Process(this->TargetOffsetAngleMin)
			.Process(this->TargetOffsetAngleMax)
			.Process(this->TargetOffsetAngleMin2)
			.Process(this->TargetOffsetAngleMax2)
			.Process(this->ReachTarget)
			.Process(this->ReachTargetEarlyEnd)
			.Process(this->ArcHeight)
			.Process(this->ArcPeakPercent)
			.Process(this->ArcPeakRandomPercent)
			.Process(this->ArcRandomHeightMin)
			.Process(this->ArcRandomHeightMax)
			.Process(this->ArcRotation)
			.Process(this->ArcRandomRotationMin)
			.Process(this->ArcRandomRotationMax)
			.Process(this->LinearSpeed)
			.Process(this->RandomSpeedMin)
			.Process(this->RandomSpeedMax)
			.Process(this->MaxSpeed)
			.Process(this->MinSpeed)
			.Process(this->Acceleration)
			.Process(this->SpeedEndOnReach)
			.Process(this->Enable)
			.Success();
	}
#pragma endregion

private:
	// ---- 字符串解析辅助（Kratos ParseMinMax / parse4 / parse4i 逐字照搬） ----

	static bool ReadStringValue(INI_EX& reader, const char* pSection, const std::string& key, std::string& out)
	{
		if (reader.ReadString(pSection, key.c_str()))
		{
			out = reader.value();
			return !out.empty();
		}
		return false;
	}

	static void ParseMinMaxInt(INI_EX& reader, const char* pSection, const std::string& key, int& min, int& max)
	{
		std::string str;
		if (!ReadStringValue(reader, pSection, key, str))
			return;
		size_t commaPos = str.find(',');
		if (commaPos != std::string::npos)
		{
			min = std::stoi(str.substr(0, commaPos));
			max = std::stoi(str.substr(commaPos + 1));
		}
		else
		{
			min = std::stoi(str);
			max = min;
		}
	}

	static void ParseMinMaxDouble(INI_EX& reader, const char* pSection, const std::string& key, double& min, double& max)
	{
		std::string str;
		if (!ReadStringValue(reader, pSection, key, str))
			return;
		size_t commaPos = str.find(',');
		if (commaPos != std::string::npos)
		{
			min = std::stod(str.substr(0, commaPos));
			max = std::stod(str.substr(commaPos + 1));
		}
		else
		{
			min = std::stod(str);
			max = min;
		}
	}

	// 4 值整数（parse4i）：min1,max1,min2,max2。双区间全无效时保留原值（回退语义）
	static void Parse4Int(INI_EX& reader, const char* pSection, const std::string& key,
		int& m1, int& M1, int& m2, int& M2)
	{
		std::string str;
		if (!ReadStringValue(reader, pSection, key, str))
			return;
		std::vector<int> v;
		std::stringstream ss(str);
		std::string t;
		while (std::getline(ss, t, ','))
			v.push_back(std::stoi(t));
		if (v.size() >= 4 && (v[0] < v[1] || v[2] < v[3]))
		{
			m1 = v[0]; M1 = v[1];
			m2 = v[2]; M2 = v[3];
		}
	}

	// 4 值浮点（parse4）：min1,max1,min2,max2，无条件覆盖
	static void Parse4Double(INI_EX& reader, const char* pSection, const std::string& key,
		double& m1, double& M1, double& m2, double& M2)
	{
		std::string str;
		if (!ReadStringValue(reader, pSection, key, str))
			return;
		std::vector<double> v;
		std::stringstream ss(str);
		std::string t;
		while (std::getline(ss, t, ','))
			v.push_back(std::stod(t));
		if (v.size() >= 4)
		{
			m1 = v[0]; M1 = v[1];
			m2 = v[2]; M2 = v[3];
		}
	}

	static bool ReadDoubles(INI_EX& reader, const char* pSection, const std::string& key, double* out, size_t maxCount)
	{
		std::string str;
		if (!ReadStringValue(reader, pSection, key, str))
			return false;
		std::vector<double> v;
		std::stringstream ss(str);
		std::string t;
		while (std::getline(ss, t, ','))
			v.push_back(std::stod(t));
		if (v.size() < maxCount)
			return false;
		for (size_t i = 0; i < maxCount; i++)
			out[i] = v[i];
		return true;
	}
};
