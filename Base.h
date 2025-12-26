#ifndef __BASE_H__
#define __BASE_H__

#include "cocos2d.h"

class Base : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);

    // implement the "static create()" method manually
    CREATE_FUNC(Base);
};
#endif // __BASE_SCENE_H__