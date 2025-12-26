#include "Base.h"
USING_NS_CC;
Scene* Base::createScene()
{
    return Base::create();
}

bool Base::init()
{
    if (!Scene::init())
    {
        return false;
    }

    // 这里可以加 Base 场景的初始化逻辑（比如加个标签测试）
    auto label = Label::createWithTTF("Base Scene", "fonts/Marker Felt.ttf", 64);
    label->setPosition(Director::getInstance()->getVisibleSize() / 2);
    this->addChild(label);

    return true;
}