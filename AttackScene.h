#ifndef __ATTACK_SCENE_H__
#define __ATTACK_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class AttackScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();

    // 注意：这里移除了menuBackCallback声明，因为我们在cpp中使用lambda函数
    // void menuBackCallback(cocos2d::Ref* pSender);

    CREATE_FUNC(AttackScene);

private:
    cocos2d::Sprite* background;
    float scaleFactor;
    bool isDragging;
    cocos2d::Vec2 lastMousePos;
    cocos2d::Vec2 backgroundPos;

    bool onMouseScroll(cocos2d::Event* event);
    bool onMouseDown(cocos2d::Event* event);
    bool onMouseMove(cocos2d::Event* event);
    bool onMouseUp(cocos2d::Event* event);
    void constrainBackgroundPosition();
};

#endif // __ATTACK_SCENE_H__