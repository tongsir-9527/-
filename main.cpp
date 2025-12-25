#include "main.h"
#include "AppDelegate.h"
#include "cocos2d.h"

USING_NS_CC;
// 这个相当于main函数
int WINAPI _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //创建app就写在这里
    AppDelegate app;
    return Application::getInstance()->run();
}
