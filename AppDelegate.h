#ifndef  _APP_DELEGATE_H_
#define  _APP_DELEGATE_H_

#include "cocos2d.h"

/**
@brief    The cocos2d Application.

Private inheritance here hides part of interface from Director.
*/
class  AppDelegate : private cocos2d::Application
{
public:
    AppDelegate();
    virtual ~AppDelegate();

    virtual void initGLContextAttrs(); //场景初始化代码在这里，初始化成功就继续执行程序
    //失败就终止程序
  
    virtual bool applicationDidFinishLaunching();
    // 程序移动到后台时调用，即点“最小化窗口”之后调用
    
    virtual void applicationDidEnterBackground();
    // 程序即将进入前台时调用，最小化窗口之后再把程序点出来的时候用

    virtual void applicationWillEnterForeground();
};

#endif // _APP_DELEGATE_H_

