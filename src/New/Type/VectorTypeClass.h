#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Constructs.h>
#include <Utilities/INIParser.h>

#include <New/Type/VectorRevibedData.h>

// ============================================================================
// VectorRevibed — 类型注册表（独立于 AE 系统，直接挂在 Projectile 上）
//
// [VectorTypes] 注册（RulesExt::Init 调用 LoadFromINIList），每个类型一个
// INI section，内部标签为 Vector.*（数据层 VectorRevibedData 解析）。
// Projectile 侧通过 BulletType 的 Vector=Vector1,Vector2 显式引用（并存叠加）。
// 段结束（Duration 到期/Deactivate）走 Vector.Next 列表：多值=分叉并存挂载，
// 首个复用本槽，其余作为新并存段并入 VectorList（Bullet Body.cpp 消费）。
// ============================================================================

// read<VectorOrigin> 特化已移至 VectorRevibedData.h（枚举定义处）：
// 本头未必被所有使用 VectorRevibedData 的编译单元包含（如 VectorRevibedState.cpp），
// 特化放枚举同文件才能保证实例化点可见，避免 C2280。

class VectorTypeClass final : public Enumerable<VectorTypeClass>
{
public:
	// --- 数据 ---
	VectorRevibedData Data;

	// --- 生命周期（脱离 AE 后自持） ---
	Valueable<int> Duration;                    // Vector.Duration，-1=无限
	ValueableIdxVector<VectorTypeClass> Next;   // Vector.Next 链式切换

	VectorTypeClass(const char* pTitle = NONE_STR) : Enumerable<VectorTypeClass>(pTitle)
		, Duration { -1 }
		, Next {}
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
