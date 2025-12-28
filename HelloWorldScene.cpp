#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"
#include "Base.h"
USING_NS_CC;//相当于using namespace std，这样就不用输入cocos2d::了

using namespace CocosDenshion; // 添加这行

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

//初始化界面函数
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    // Clash Of Clans 标题
    auto label = Label::createWithTTF("Clash Of Clans", "fonts/Marker Felt.ttf", 248);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height - label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(label, 1);
    }

    // 部落冲突封面
    auto sprite = Sprite::create("ClashOfClansCover.png");
    if (sprite == nullptr)
    {
        problemLoading("'ClashOfClansCover.png'");
    }
    else
    {
        // position the sprite on the center of the screen
        sprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }

    // 开始游戏按钮
    auto startButton = ui::Button::create("StartGame.png");
    if (startButton == nullptr)
    {
        problemLoading("'StartGame.png'");
    }
    else
    {
        // 设置按钮位置（屏幕下方偏中）
        startButton->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 4));

        // 点击后跳转到 Base 场景
        startButton->addClickEventListener([](Ref* sender) {
            // 停止当前背景音乐
            SimpleAudioEngine::getInstance()->stopBackgroundMusic();

            // 创建 Base 场景
            auto baseScene = Base::createScene();
            if (baseScene)
            {
                // 场景切换，时间0.5秒
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, baseScene));
            }
            });

        // 将按钮添加到场景中
        this->addChild(startButton, 1); // zOrder 设为 1，确保在封面图上方显示
    }

    // 播放背景音乐
    SimpleAudioEngine::getInstance()->playBackgroundMusic("audio/Start.mp3", true);

    return true;
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    // 停止背景音乐
    SimpleAudioEngine::getInstance()->stopBackgroundMusic();

    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}