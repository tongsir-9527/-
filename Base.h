#ifndef __BASE_H__
#define __BASE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Architecture.h"  // 添加建筑类头文件

class Base : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    void menuCloseCallback(cocos2d::Ref* pSender);
    void menuBackCallback(cocos2d::Ref* pSender);

    bool onMouseScroll(cocos2d::Event* event);
    bool onMouseDown(cocos2d::Event* event);
    bool onMouseMove(cocos2d::Event* event);
    bool onMouseUp(cocos2d::Event* event);

    CREATE_FUNC(Base);

private:
    cocos2d::Sprite* background;
    float scaleFactor;
    bool isDragging;
    cocos2d::Vec2 lastMousePos;
    cocos2d::Vec2 backgroundPos;
    void constrainBackgroundPosition();

    // 资源变量
    int _gold;
    int _elixir;
    int _darkElixir;

    // 建筑指针（已修正声明）
    Architecture* _commandCenter;  // 这里需要确保Architecture类已声明
};

#endif // __BASE_H__