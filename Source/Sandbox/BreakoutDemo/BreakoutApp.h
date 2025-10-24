#pragma once
#include "../Bamboo/Game/Application.h"
#include<string>

class BreakoutApp :public Bamboo::Application
{
public:
    BreakoutApp(const std::string& appName = "BreakoutApp");
    ~BreakoutApp();
};


