/**
 * @file EntityValiditySelfTest.h
 * @brief 临时自检：验证 Entity 的效性检查是否生效（docs/README.md 缺陷 P0-5）。
 *
 * 为什么单独放一个文件而不是写在 main.cpp 里：
 *   验证完就删掉/不 include 即可，不会常驻在正式入口里。
 *
 * 用法（两行）：
 *   1) #include "EntityValiditySelfTest.h"
 *   2) 在 app.Run() **之前** 插入：SelfTest_EntityValidity(*app.GetSceneManager()->GetActiveScene());
 *
 * 期望输出（走 stderr，和引擎日志一起看）：
 *   [self-test] 默认构造: IsValid=0 bool=0  (期望 0 0)
 *   [self-test] 查不到:   IsValid=0 HasTransform=0  (期望 0 0)
 *   [self-test] 空实体 GetComponent ... 未崩溃 OK
 *   [self-test] 存在的实体: IsValid=1 HasCamera=1  (期望 1 1)
 *
 * 如果第 1/2/4 行的值与期望不符，或程序在"空实体 GetComponent"处崩溃，
 * 说明 P0-5 没有修好。
 */

#pragma once

#include "../Bamboo/ECS/Entity.h"
#include "../Bamboo/Scene/Scene.h"

#include <iostream>

inline void SelfTest_EntityValidity(Bamboo::Scene &scene)
{
    using Bamboo::Entity;
    std::ostream &out = std::cerr; // 无缓冲，便于在 VS 输出窗口/重定向里立刻看到

    // 1) 默认构造的实体必须是无效的，且不能崩
    Entity empty{};
    out << "[self-test] 默认构造: IsValid=" << empty.IsValid()
        << " bool=" << (bool)empty << "  (期望 0 0)\n";

    // 2) 查一个不存在的名字 → 返回空实体（这是过去会崩的路径）
    Entity missing = scene.FindEntityByName("NoSuchEntity");
    out << "[self-test] 查不到:   IsValid=" << missing.IsValid()
        << " HasTransform=" << missing.HasComponent<Bamboo::TransformComponent>() << "  (期望 0 0)\n";

    // 3) 对空实体调用 GetComponent：Debug 下会打一条断言日志，但不应崩溃
    out << "[self-test] 空实体 GetComponent ... " << std::flush;
    (void)empty.GetComponent<Bamboo::TransformComponent>();
    out << "未崩溃 OK\n";

    // 4) 正常实体必须仍然有效
    Entity camera = scene.FindEntityByName("MainCamera");
    out << "[self-test] 存在的实体: IsValid=" << camera.IsValid()
        << " HasCamera=" << camera.HasComponent<Bamboo::CameraComponent>() << "  (期望 1 1)\n";
}
