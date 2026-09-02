#pragma once
// ============================================================================
// VectorRevibed — 逻辑层（Kratos VectorEffectReVibed 移植到 PhobosAI）
//
// 轨迹算法铁律：数学公式、帧时序、随机分布语义与 Kratos 新版逐字一致。
// 适配点（仅基建）：
//   - EffectScript 生命周期 → 自由函数 Init/Step + 显式参数（pObject/pLauncher/pSource/duration）
//   - TechnoStatus 目标缓存 → 砍掉（用户确认），GetTargetPosFromChain 直接走引擎链
//   - 本地 mt19937 → 引擎同步随机（ScenarioClass::Instance->Random）
//   - Deactivate() → out.Deactivate 标志（调用方处理 Next 列表/移除）
//   - 随机取整：lround + Y 镜像 原样（GetFLHAbsoluteOffset 本地实现）
// ============================================================================

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include <GeneralDefinitions.h>
#include <Fundamentals.h>
#include <YRMath.h>

#include <Matrix3D.h>

#include <ScenarioClass.h>

#include <Utilities/Stream.h>

#include <New/Type/VectorRevibedData.h>

// MSVC <cmath> 默认不定义 M_PI（Kratos VectorDataReVibed.h 同款保护）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// 位移结果（Kratos VectorResult 原样 + Deactivate 信号）
// ============================================================================
struct VectorRevibedResult
{
	CoordStruct MoveDisp = CoordStruct::Empty; // 位移向量
	bool CanPassBuilding = false;              // 是否可以穿过建筑（预留，未消费）
	bool Freeze = false;                       // 是否冻结标志位，如果为true，则表示被冻结，位移无效
	CoordStruct FrozenPos = CoordStruct::Empty;// 冻结时的位置，仅当Freeze为true时有效
	bool Force = false;                        // Force 模式，Vector 完全接管位置（非运动帧也保持原地）
	bool AllowCrawl = false;                   // 是否播放爬行帧（预留，未消费）
	bool AllowRotateUnit = false;              // 是否调整朝向
	CoordStruct TeleportTo = CoordStruct::Empty;// 新增（PhobosAI）：SpeedEndOnReach 到达帧请求瞬移的目标点；
												// 由调用方在"唯一活跃 Vector"时应用（并存时忽略，该 Vector 结束引擎接管）
	bool Deactivate = false;                   // 新增（PhobosAI）：请求结束本段 Vector（Next 链/移除）

	// --- Phobos 存档 ---
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) { return this->Serialize(Stm); }
	bool Save(PhobosStreamWriter& Stm) const { return const_cast<VectorRevibedResult*>(this)->Serialize(Stm); }
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->MoveDisp)
			.Process(this->Freeze)
			.Process(this->FrozenPos)
			.Process(this->Force)
			.Process(this->AllowRotateUnit)
			.Process(this->TeleportTo)
			.Process(this->Deactivate)
			.Success();
	}
};

// ============================================================================
// 运动状态（主/大圆共用结构；持两份：_motion + _originMotion）
// ============================================================================
enum class VectorRevibedMotionKind : int
{
	None = 0,
	MoveTo,        // 纯 FLH 位移 + GrowRate + AnglePerStep 自旋
	ReachTarget,   // 剩余帧数均分位移，强制到达
	Speed,         // 直线追踪 + 加速度 + 影子坐标弧高
	Circle,        // 圆周运动（三选二参数）
};

struct VectorRevibedMotionState
{
	// --- 运动进度 ---
	int elapsed = 0;                // 已执行运动帧数（主=_movementFrames 同源，大圆独立）
	double speed = 0.0;             // 当前线速度（含加速度累加）
	double angle = 0.0;             // 当前角度：MoveTo 自旋 / Circle 相位
	double circleRadius = 0.0;      // Circle 动态半径
	double circleSpeed = 0.0;       // Circle 线速度（含加速度累加）
	double circleAngle = 0.0;       // Circle 角速度（含加速度累加）

	// --- 倾斜圆面法线（NormalVector 系统）---
	double normalRotF = 0.0;        // 法线绕 F 轴累计旋转（Lissajous 累加用）
	double normalStepF = 0.0;       // 法线每步角速度（已解析：常数/区间随机）
	double normalStepL = 0.0;
	double normalStepH = 0.0;
	double normalX = 0.0, normalY = 0.0, normalZ = 1.0; // 3D 法向量（世界坐标，增量旋转维护）
	double lissajousStep = 0.0;     // 圆周 F 偏移角速度（°/step），0=不偏移

	// --- 弧线（ReachTarget/Speed）---
	double arcHeight = 0.0;         // 弧高（Init 解析随机后写入）
	double arcPeakPercent = 0.5;    // 弧高点比率 0..1
	double arcRotation = 0.0;       // 弧面旋转角（°），0=朝上

	// --- Speed 影子坐标（弧高进度基准，不受弧高 Z 偏移污染）---
	double shadowX = 0.0;
	double shadowY = 0.0;
	double shadowZ = 0.0;
	double shadowTraveled = 0.0;    // 影子累计行走距离（含加速度/变速）
	double prevArcOffset = 0.0;     // 上一帧弧高绝对值（增量叠加用）

	// --- 大圆 Speed 弧高专用（主模式不用）---
	double arcTotalDist = -1.0;     // 首帧初始总距离（<0=未初始化）
	CoordStruct arcStartCenter{};   // 弧线起始圆心位置

	// --- Phobos 存档（Kratos 的 Process 链 + Phobos Load/Save 包装） ---
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) { return this->Process(Stm); }
	bool Save(PhobosStreamWriter& Stm) const { return const_cast<VectorRevibedMotionState*>(this)->Process(Stm); }
	template <typename T>
	bool Process(T& stream)
	{
		return stream
			.Process(this->elapsed)
			.Process(this->speed)
			.Process(this->angle)
			.Process(this->circleRadius)
			.Process(this->circleSpeed)
			.Process(this->circleAngle)
			.Process(this->normalRotF)
			.Process(this->normalStepF)
			.Process(this->normalStepL)
			.Process(this->normalStepH)
			.Process(this->normalX)
			.Process(this->normalY)
			.Process(this->normalZ)
			.Process(this->lissajousStep)
			.Process(this->arcHeight)
			.Process(this->arcPeakPercent)
			.Process(this->arcRotation)
			.Process(this->shadowX)
			.Process(this->shadowY)
			.Process(this->shadowZ)
			.Process(this->shadowTraveled)
			.Process(this->prevArcOffset)
			.Process(this->arcTotalDist)
			.Process(this->arcStartCenter)
			.Success();
	}
};

// ============================================================================
// 运行时状态（对应 Kratos VectorEffect 全部成员）
// ============================================================================
struct VectorRevibedState
{
	// --- 计时/帧 ---
	int _elapsedFrames = 0;             // 已执行运动帧数（含 TimeStep 跳帧）
	int _moveFrame = 0;                 // 真实帧计数
	int _movementFrames = 0;            // 有效运动帧数（不含 DisabledFrames/TimeStep 跳帧）
	int _effectiveTimeStep = 1;         // 有效 TimeStep
	int _totalDuration = 0;             // 总持续时间（ReachTarget 用，已除 TimeStep）
	int _lifeFrames = 0;                // 段真实帧计时：VectorAI 每帧无条件推进（Freeze/Disabled/
										// TimeStep 跳帧期间也走）。Duration 到期据此强制结束本段——
										// 运动帧计数（_elapsedFrames）在 Freeze 分支不推进，
										// 不能作为到期时钟（Kratos 由 AE 层倒计时，移植后归此）

	// --- 快照/引用 ---
	CoordStruct _initialLocation{};     // 首帧位置快照（弧线基准/Freeze 锚点）
	CoordStruct _initialOriginPos{};    // 主 Origin 最后有效坐标（Init 锁定 + 每帧跟随）
	CoordStruct _initialBaseCenter{};   // 大圆基座最后有效坐标（OriginOriginNoUpdate 冻结用）
	int _vectorAcquireZ = 0;            // 获取 Vector 时的抛射体 Z（Circle 圆心高度基准）
	ObjectClass* _pLauncher = nullptr;  // 发射者（每帧使用前判空）
	ObjectClass* _pSource = nullptr;    // AE 来源（PhobosAI 无 AE，bullet 侧 = 空/发射者）

	// --- 朝向（主模式参考系）---
	double _facingRad = 0.0;            // Init 锁定的朝向弧度（FLH 旋转用）
	DirStruct _facingDir;               // Init 锁定的朝向（Point2Dir 结果，Target/Source）
	double _tiltRad = 0.0;              // F 轴俯仰角（AllowCircleTilt 用）

	// --- 大圆朝向（独立参考系）---
	double _originFacing = 0.0;         // 大圆有效 facing
	double _originTilt = 0.0;           // 大圆有效 tilt
	// 首帧锁定的基础法向量球坐标（OriginNormalVector/OriginNormalRandom/默认水平）：
	double _baseOriginFacing = 0.0;
	double _baseOriginTilt = M_PI / 2.0;

	// --- 目标偏移 ---
	CoordStruct _randomTargetOffset{};  // 主 TargetFLH 随机偏移（首帧解析）
	CoordStruct _originTargetOffset{};  // 大圆 TargetFLH 随机偏移

	// --- 大圆圆心运动 ---
	CoordStruct _originOffset{};        // 圆心相对基座偏移（首帧 0，每帧累加 disp）
	CoordStruct _prevCircleCenter{};    // 上一帧圆心位置（计算叠加位移用）
	CoordStruct _circlePos{};           // 圆上内部跟踪位置（增量位移，不打架 MoveTo）

	// --- 运动状态（主 + 大圆）---
	VectorRevibedMotionState _motion{};      // 主运动状态
	VectorRevibedMotionState _originMotion{};// 大圆圆心运动状态

	// --- 帧工具 ---
	bool ShouldMoveThisFrame() const
	{
		return (_moveFrame % _effectiveTimeStep) == 0;
	}
	void AdvanceFrame()
	{
		_elapsedFrames++;
		_moveFrame++;
	}

	// --- Phobos 存档 ---
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) { return this->Serialize(Stm); }
	bool Save(PhobosStreamWriter& Stm) const { return const_cast<VectorRevibedState*>(this)->Serialize(Stm); }
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->_elapsedFrames)
			.Process(this->_moveFrame)
			.Process(this->_movementFrames)
			.Process(this->_effectiveTimeStep)
			.Process(this->_totalDuration)
			.Process(this->_initialLocation)
			.Process(this->_initialOriginPos)
			.Process(this->_initialBaseCenter)
			.Process(this->_vectorAcquireZ)
			.Process(this->_pLauncher)
			.Process(this->_pSource)
			.Process(this->_facingRad)
			.Process(this->_facingDir)
			.Process(this->_tiltRad)
			.Process(this->_originFacing)
			.Process(this->_originTilt)
			.Process(this->_baseOriginFacing)
			.Process(this->_baseOriginTilt)
			.Process(this->_randomTargetOffset)
			.Process(this->_originTargetOffset)
			.Process(this->_originOffset)
			.Process(this->_prevCircleCenter)
			.Process(this->_circlePos)
			.Process(this->_motion)
			.Process(this->_originMotion)
			.Process(this->_lifeFrames) // 追加于链尾：旧档缺省回退 0（重新计时）
			.Success();
	}
};

// ============================================================================
// 入口（自由函数，由 Bullet 侧每帧调用）
// ============================================================================
// Init：挂载/链切换时调用一次，解析数据 → 运行时参数（对应 Kratos OnStart）
void VectorRevibedAI_Init(VectorRevibedState& s, const VectorRevibedData& d,
	ObjectClass* pObject, ObjectClass* pLauncher, ObjectClass* pSource, int duration);

// Step：每帧计算位移（对应 Kratos GetVectorResult），结果写 out
void VectorRevibedAI_Step(VectorRevibedState& s, const VectorRevibedData& d,
	ObjectClass* pObject, ObjectClass* pLauncher, ObjectClass* pSource, int duration,
	VectorRevibedResult& out);
