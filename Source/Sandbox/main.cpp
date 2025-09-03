/**
 * @file main.cpp
 * @brief 测试文件
 */
#include <iostream>
#include<filesystem>
#include "../Bamboo/Game/Application.h";
#include "../Bamboo/Scene/Scene.h";
#include "../Bamboo/Scene/SceneManager.h"

#include "../Bamboo/ECS/Entity.h";
#include "../Bamboo/ECS/Component/Component.h"

#include "../Bamboo/Math/Color.h";


int main(int argc, char** argv) {
    Bamboo::Application app;
    
    auto entity = app.GetSceneManager()->GetActiveScene()->CreateEntity();
    auto& component = entity.AddComponent<Bamboo::TriangleComponent>();
    component.TriangleColor = Bamboo::Color::Blue;
    
    app.Run();

    std::cout << std::filesystem::current_path() << std::endl;

    return 0;
}