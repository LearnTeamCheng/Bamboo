/**
 * @file main.cpp
 * @brief Sandbox 示例程序入口
 *
 * 目前只启动 Breakout 示例；早先那些用来手工验证渲染/纹理的临时代码
 * （直接构造 Application 摆几个精灵）已删除，需要时请看 git 历史。
 */

#include "BreakoutDemo/BreakoutApp.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

int main(int argc, char **argv)
{
#ifdef _DEBUG
    // 开启 CRT 内存检测：程序退出时自动 dump 泄漏。
    // 需要在 VS 输出窗口查看；配合 _CRTDBG_MAP_ALLOC 还能显示泄漏点的文件与行号。
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    BreakoutApp app;
    app.Run();

    return 0;
}
