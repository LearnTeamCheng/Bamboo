#include "BreakoutApp.h"
#include "../Bamboo/ECS/Entity.h"
//using namespace Bamboo;

BreakoutApp::BreakoutApp(const std::string &appName) : Application(appName)
{
    auto &entity = GetSceneManager()->GetActiveScene()->CreateEntity();
}

BreakoutApp::~BreakoutApp() {}