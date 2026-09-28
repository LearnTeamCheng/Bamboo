#pragma once

#include <variant>
#include <string>
#include <cstdint>

#include "../UUID.h"
#include "../../Math/Vector2.h"
#include "../../Math/Vector3.h"
#include "../../Math/Vector4.h"
#include "../../Math/Color.h"
#include "../../Assets/AssetHandle.h"

namespace Bamboo
{
    /// @brief 字段值的通用载体。
    ///
    /// 为什么需要它：Property::GetPtr() 返回的 void* 只能"原地访问"，
    /// 无法取出、传递、比较、放进容器。而泛型消费者需要后者：
    ///   - 编辑器检查器：拿到值 → 决定画什么控件
    ///   - 剪贴板：把任意组件的字段拷贝出来
    ///   - 序列化：把值写进 JSON
    ///   - C# 绑定：按字段名读写
    ///
    /// 注意 std::monostate 必须放在第一位：
    /// std::variant 的默认构造是"第一个备选"，如果第一个是 bool，
    /// 默认值会变成 false（一个有效的值）而不是"无值"。
    using PropertyValue = std::variant<
        std::monostate, // 无值 / 未设置
        bool,
        int32_t,
        uint32_t,
        float,
        double,
        std::string,
        Vector2,
        Vector3,
        Vector4,
        Color,
        UUID,
        AssetHandle>;

    /// @brief 便捷判断：这个值是不是"无值"
    inline bool IsEmpty(const PropertyValue &value)
    {
        return std::holds_alternative<std::monostate>(value);
    }
}
