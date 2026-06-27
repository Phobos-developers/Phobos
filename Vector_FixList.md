# Phobos Vector 待修问题清单
> 基于 KratosAI `src/Ext/EffectType/Effect/VectorEffect.cpp` 与 PhobosAI `src/New/Entity/VectorState.cpp` 逐行对比。
> 所有行号以当前 feature/vector-port 分支为准。

---

## 🔴 B1 — OriginNormalVector atan2 参数反了
**文件**: `src/New/Entity/VectorState.cpp:419`  
**问题**: `std::atan2(nv.Y, nv.X)` — 参数顺序是 `atan2(L, F)`，应为 `atan2(F, L)`。  
**参考 Kratos**: `VectorEffect.cpp:587` — `std::atan2(fy, fx)` where `fy = OriginNormalVector.X (F), fx = .Y (L)`  
**修复**: 改为 `std::atan2(nv.X, nv.Y)`  
**影响**: 原点法向量旋转 90°，圆心方向错误。

---

## 🔴 B2 — pInvoker 存活检查缺失
**文件**: `src/New/Entity/VectorState.cpp:256`  
**问题**: `auto const pLT = static_cast<TechnoClass*>(pInvoker)` 无 null/存活检查，pInvoker 死亡时直接崩溃。  
**参考 Kratos**: `VectorEffect.cpp:400-402` — `if (_pLauncher && !IsDeadOrInvisible(_pLauncher)) { TechnoClass* pLauncherTechno = abstract_cast<TechnoClass*>(_pLauncher); }`  
**修复**: 在 `static_cast` 前加 `if (pInvoker && !pInvoker->IsAlive)` 守卫，或换成 `abstract_cast`。

---

## 🔴 B3 — 动态 facing 地面子弹无 TargetCoords 回退
**文件**: `src/New/Entity/VectorState.cpp:211`  
**问题**: `if (pB->Target)` 只检查 Techno* 目标，地面对 TargetCoords 无回退。  
**参考 Kratos**: `VectorEffect.cpp:364-371` — 区分 ground 和 Techno，ground 用 `TargetCoords`。  
**修复**: 改为 `pB->Target ? pB->Target->GetCoords() : pB->TargetCoords`

---

## 🟡 B4 — Origin=Launcher/Target/Source 初始 facing 缺失
**文件**: `src/New/Entity/VectorState.cpp:87-104`  
**问题**: 初始化时统一用 bullet velocity 或 PrimaryFacing，未按 Origin 类型分别设置 facing。  
**参考 Kratos**: `VectorEffect.cpp:180-259` — 完整的 switch(Origin) 分支，Launcher 用发射者朝向，Target 用自→目标方向，Source 用源→自方向，均含 AllowedTilt tilt 计算。  
**修复**: 在 `s.Initialized` 块中，模仿 Kratos 的 switch 分支设置 `s.FacingRad` 和 `s.TiltRad`（用于 OriginNoUpdate 或无 NormalVector 的初始 facing）。

---

## 🟡 B5 — 初始速度无 techno/bullet 回退
**文件**: `src/New/Entity/VectorState.cpp:106`  
**问题**: `InitialSpeed >= 0 ? InitialSpeed : 0` — 默认 0，不读单位/弹体原生速度。  
**参考 Kratos**: `VectorEffect.cpp:60-71` — 未设 InitialSpeed 时回退到 `pType->JumpjetSpeed / Speed` 或 `pBullet->Speed`。  
**修复**: InitialSpeed < 0 时从 pObject 读原生速度。

---

## 🟡 B6 — Self 模式 InitialFacing 用 Primary 而非 Turret
**文件**: `src/New/Entity/VectorState.cpp:100`  
**问题**: `pT->PrimaryFacing.Current().GetRadian<32>()` 应改为 `TurretFacing()`.  
**参考 Kratos**: `VectorEffect.cpp:389` — 有 `OriginIsOnBody` 控制，默认 TurretFacing。  
**修复**: 如 OriginIsOnBody=false，用 `pT->TurretFacing().Current()`。

---

## 🟡 B7 — remainingFrames 差 1 帧
**文件**: `src/New/Entity/VectorState.cpp:687`  
**问题**: `effectiveDuration - s.MovementFrames` 应改为 `effectiveDuration - s.MovementFrames + 1`。  
**参考 Kratos**: `VectorEffect.cpp:930` — `effectiveDuration - _movementFrames + 1`。  
**修复**: 加 `+1`。

---

## 🟡 B8 — Source+OriginNoUpdate 初始位置未初始化
**文件**: `src/New/Entity/VectorState.cpp:58-70`  
**问题**: `s.InitialOriginPos` 只处理了 Launcher 和 Target 分支，无 Source 分支。  
**参考 Kratos**: `VectorEffect.cpp:116-124` — Source 时读 `AE->pSource->GetCoords()`。  
**修复**: 添加 `case VectorOrigin::Source:` 分支，从 pInvoker (对子弹即为 AE source) 取坐标。

---

## 🟠 M1 — 圆随机半径用 int trunc
**文件**: `src/New/Entity/VectorState.cpp:325`  
**问题**: `rand() % (Max - Min + 1)` 整数截断。  
**参考 Kratos**: `VectorEffect.cpp:780` — `Random::RandomRanged` (double)。  
**修复**: 改用 `V_Random(Min, Max)`。

---

## 🟠 M2 — OriginLissajous 缺失
**文件**: `src/New/Entity/VectorState.cpp:428-442` (圆形移动段)  
**问题**: Phobos 始终用 smooth 增量旋转，无 Lissajous 模式。  
**参考 Kratos**: `VectorEffect.cpp:703-704` — `Data->OriginLissajous ? cumulativeAngle : incrementOnly`。  
**修复**: 添加 `Vector_OriginLissajous` 标签，按 Kratos 逻辑分支处理。

---

## 🟠 M3 — Origin ReachTarget + ArcHeight 缺失
**文件**: `src/New/Entity/VectorState.cpp:407-455`  
**问题**: 圆心只支持 Speed 模式（恒速追踪），无 ReachTarget 模式及弧高。  
**参考 Kratos**: `VectorEffect.cpp:654-679` — OriginReachTarget 分支含 remainingFrames 分批到达及 `OriginArcHeight` 弧高计算。  
**修复**: 添加完整的 OriginReachTarget 分支代码。

---

## 🟠 M4 — OriginCircleOffset 缺失
**问题**: Phobos 无 `Vector_OriginCircleOffset` 标签。  
**参考 Kratos**: `VectorEffect.cpp:559-560` — `baseCenter += Data->OriginCircleOffset` (世界偏移)。  
**修复**: 新增标签及读取，在 baseCenter 计算后叠加。

---

## 🟠 M5 — OriginTargetOffset 随机化缺失
**文件**: `src/New/Entity/VectorState.cpp:413`  
**问题**: `s.OriginTargetOffset = CoordStruct::Empty` 未赋值。  
**参考 Kratos**: `VectorEffect.cpp:576-578` — 每轴独立随机范围。  
**修复**: 初始化时按 `OriginTargetOffsetF/L/H Min/Max` 随机赋值。

---

## 🟠 M6 — Origin 加速度/钳位缺失
**问题**: Phobos 圆心速度恒定为 InitialSpeed，无加速度和上下限。  
**参考 Kratos**: `VectorEffect.cpp:683-685` — `OriginAcceleration`、`OriginMaxSpeed`、`OriginMinSpeed`。  
**修复**: 新增标签，Speed 模式每帧叠加加速度并钳位。

---

## 🟠 M7 — NormalCircleRadius 哨兵缺失
**问题**: 有 NormalVector 但无明确 CircleRadius 时不会自动推断。  
**参考 Kratos**: `VectorEffect.cpp:595-596` — 设 `_originCircleRadius = effectiveFacing > -1 ? 0 : -1` 作为哨兵。  
**修复**: 在初始化时加类似哨兵逻辑。

---

## 🟠 M8 — ReachTargetEarlyEnd 死循环
**文件**: `src/New/Entity/VectorState.cpp:689-693`  
**问题**: DisabledTimer 归零后 remainingFrames 不变，反复触发 EarlyEnd。  
**参考 Kratos**: `VectorEffect.cpp:934` — `Deactivate()` 直接终止，不设 timer。  
**修复**: 改为一次性 `s.DisabledTimer = EarlyEnd` 后立即递增 `s.MovementFrames` 以退出循环区间，或直接 `Deactivate()`。

---

## 🟠 M9 — 建筑对象无排除
**文件**: `src/New/Entity/VectorState.cpp:33` (VectorAI_Run 入口)  
**问题**: 无 building 类型检查。  
**参考 Kratos**: `VectorEffect.cpp:19-23` — Building 对象直接 `Deactivate()`。  
**修复**: 入口加 `if (pObject->WhatAmI() == AbstractType::Building) return;`

---

## 🟠 M10 — 圆模式双区间随机角速度缺失
**问题**: Phobos 只有单区间 `CircleRandomAngleMin/Max`。  
**参考 Kratos**: `VectorEffect.cpp:485-489` — 4 值区间 `CircleRandomAngleMin/Max` + `Min2/Max2`。  
**修复**: 新增标签及初始化时的 4 值随机逻辑。

---

## 📋 修复顺序建议
1. 先修 🔴 B1~B3（崩溃/功能错误）
2. 再修 🟡 B4~B8（行为偏差）
3. 最后补 🟠 M1~M10（缺失功能）

每次修改后验证编译 `scripts\build_debug.bat` 通过。
