#include "AttackScene.h"
#include "Base.h"
#include "HelloWorldScene.h"
USING_NS_CC;
using namespace cocos2d::ui;

Scene* AttackScene::createScene()
{
    return AttackScene::create();
}

bool AttackScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    scaleFactor = 1.0f;
    isDragging = false;

    // 创建背景
    background = Sprite::create("base_background.png");
    if (background)
    {
        background->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        backgroundPos = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
        background->setPosition(backgroundPos);
        background->setScale(scaleFactor);
        this->addChild(background, 0);
    }

    // 场景标题
    auto label = Label::createWithTTF("Attack Scene", "fonts/Marker Felt.ttf", 64);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - label->getContentSize().height));
    this->addChild(label, 1);

    // 返回按钮（左上角）- 修复：确保按钮正常工作
    auto backButton = Button::create("BackButton.png");
    if (backButton)
    {
        backButton->setPosition(Vec2(origin.x + backButton->getContentSize().width / 2 + 20,
            origin.y + visibleSize.height - backButton->getContentSize().height / 2 - 20));
        backButton->setTag(100); // 设置tag

        // 修复：确保点击事件被正确处理 - 直接使用lambda函数
        backButton->addClickEventListener([this](Ref* sender) {
            CCLOG("AttackScene返回按钮被点击");
            auto baseScene = Base::createScene();
            if (baseScene)
            {
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, baseScene));
            }
            });

        this->addChild(backButton, 5);
    }

    // 鼠标事件监听器（用于地图拖动和缩放）
    auto listener = EventListenerMouse::create();
    listener->onMouseScroll = CC_CALLBACK_1(AttackScene::onMouseScroll, this);
    listener->onMouseDown = CC_CALLBACK_1(AttackScene::onMouseDown, this);
    listener->onMouseMove = CC_CALLBACK_1(AttackScene::onMouseMove, this);
    listener->onMouseUp = CC_CALLBACK_1(AttackScene::onMouseUp, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

bool AttackScene::onMouseScroll(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (!background) return false;

    float scrollY = e->getScrollY();

    if (scrollY > 0)
    {
        scaleFactor *= 0.9f;
        if (scaleFactor < 1.0f)
            scaleFactor = 1.0f;
    }
    else if (scrollY < 0)
    {
        scaleFactor *= 1.1f;
        if (scaleFactor > 3.0f)
            scaleFactor = 3.0f;
    }

    background->setScale(scaleFactor);
    constrainBackgroundPosition();

    return true;
}

bool AttackScene::onMouseMove(Event* event)
{
    if (!isDragging) return false;

    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 delta = currentMousePos - lastMousePos;

    if (background) {
        Vec2 newPos = backgroundPos + delta;
        backgroundPos = newPos;
        constrainBackgroundPosition();
        background->setPosition(backgroundPos);
    }

    lastMousePos = currentMousePos;
    return true;
}

void AttackScene::constrainBackgroundPosition()
{
    if (!background) return;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    const float originalBgWidth = 1824.0f;
    const float originalBgHeight = 1398.0f;

    float scaledBgWidth = originalBgWidth * scaleFactor;
    float scaledBgHeight = originalBgHeight * scaleFactor;

    float gameMinX = origin.x;
    float gameMaxX = origin.x + visibleSize.width;
    float gameMinY = origin.y;
    float gameMaxY = origin.y + visibleSize.height;

    float minX = gameMinX + scaledBgWidth / 2;
    float maxX = gameMaxX - scaledBgWidth / 2;
    float minY = gameMinY + scaledBgHeight / 2;
    float maxY = gameMaxY - scaledBgHeight / 2;

    backgroundPos.x = clampf(backgroundPos.x, minX, maxX);
    backgroundPos.y = clampf(backgroundPos.y, minY, maxY);
    background->setPosition(backgroundPos);
}

bool AttackScene::onMouseDown(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 mousePos = this->convertToNodeSpace(Vec2(e->getCursorX(), e->getCursorY()));

    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        // 检查是否点击了返回按钮
        auto backButton = dynamic_cast<Button*>(this->getChildByTag(100));
        if (backButton && backButton->getBoundingBox().containsPoint(mousePos)) {
            // 按钮点击已经在addClickEventListener中处理
            return true;
        }

        isDragging = true;
        lastMousePos = mousePos;
    }
    return true;
}

bool AttackScene::onMouseUp(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);

    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        isDragging = false;
    }
    return true;
}