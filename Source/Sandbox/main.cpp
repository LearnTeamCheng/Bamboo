/**
 * @file main.cpp
 * @brief 测试文件
 */

#include <iostream>
#include "../Bamboo/Game/Application.h";
#include "../Bamboo/Scene/Scene.h";
#include "../Bamboo/Scene/SceneManager.h"

#include "../Bamboo/ECS/Entity.h";
#include "../Bamboo/ECS/Component/Component.h"


int main(int argc, char** argv) {
    Bamboo::Application app;
    
    auto entity = app.GetSceneManager()->GetActiveScene()->CreateEntity();
    entity.AddComponent<Bamboo::TriangleComponent>();
    
    app.Run();



    return 0;
}