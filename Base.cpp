#include "Base.h"
#include "HelloWorldScene.h"
#include "Architecture.h"
USING_NS_CC;
using namespace cocos2d::ui;

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

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    scaleFactor = 1.0f; // 初始缩放为1倍
    isDragging = false;

    // 添加背景图
    background = Sprite::create("base_background.png");
    if (background)
    {
        background->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        // 初始位置设为屏幕中心
        backgroundPos = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
        background->setPosition(backgroundPos);
        background->setScale(scaleFactor);
        this->addChild(background, 0);
    }

    // 添加标题
    auto label = Label::createWithTTF("Base Scene", "fonts/Marker Felt.ttf", 64);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - label->getContentSize().height));
    this->addChild(label, 1);

    // 添加返回按钮
    auto backButton = Button::create("BackButton.png");
    if (backButton)
    {
        backButton->setPosition(Vec2(origin.x + backButton->getContentSize().width / 2 + 20,
            origin.y + visibleSize.height - backButton->getContentSize().height / 2 - 20));
        backButton->addClickEventListener(CC_CALLBACK_1(Base::menuBackCallback, this));
        this->addChild(backButton, 1);
    }

    // 添加关闭按钮
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(Base::menuCloseCallback, this));

    if (closeItem)
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));

        auto menu = Menu::create(closeItem, NULL);
        menu->setPosition(Vec2::ZERO);
        this->addChild(menu, 1);
    }

    // 注册鼠标事件监听器
    auto listener = EventListenerMouse::create();
    listener->onMouseScroll = CC_CALLBACK_1(Base::onMouseScroll, this);
    listener->onMouseDown = CC_CALLBACK_1(Base::onMouseDown, this);
    listener->onMouseMove = CC_CALLBACK_1(Base::onMouseMove, this);
    listener->onMouseUp = CC_CALLBACK_1(Base::onMouseUp, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 初始化资源
    _gold = 0;
    _elixir = 0;
    _darkElixir = 0;

    // 创建司令部(决定其他建筑等级上限)
    _commandCenter = Architecture::create(BuildingType::COMMAND_CENTER, 1);
    if (_commandCenter) {
        _commandCenter->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        this->addChild(_commandCenter, 1);
    }

    // 创建金矿
    auto goldMine = Architecture::create(BuildingType::GOLD_MINE, 1);
    if (goldMine) {
        goldMine->setPosition(Vec2(visibleSize.width / 3, visibleSize.height / 2));
        goldMine->setResourceCallback([this](ResourceType type, int amount) {
            _gold += amount;
            CCLOG("Gold: %d", _gold);  // 打印资源变化
            });
        this->addChild(goldMine, 1);
    }

    // 创建圣水收集器
    auto elixirCollector = Architecture::create(BuildingType::ELIXIR_COLLECTOR, 1);
    if (elixirCollector) {
        elixirCollector->setPosition(Vec2(visibleSize.width * 2 / 3, visibleSize.height / 2));
        elixirCollector->setResourceCallback([this](ResourceType type, int amount) {
            _elixir += amount;
            CCLOG("Elixir: %d", _elixir);  // 打印资源变化
            });
        this->addChild(elixirCollector, 1);
    }

    return true;
}

void Base::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}

void Base::menuBackCallback(Ref* pSender)
{
    auto helloWorldScene = HelloWorld::createScene();
    if (helloWorldScene)
    {
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, helloWorldScene));
    }
}

// 鼠标滚轮缩放处理（向上滚放大，向下滚缩小，范围1-3倍）
bool Base::onMouseScroll(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (!background) return false;

    float scrollY = e->getScrollY();

    // 向上滚动放大
    if (scrollY > 0)
    {
        scaleFactor *= 1.1f;
        if (scaleFactor > 3.0f) // 最大3倍
            scaleFactor = 3.0f;
    }
    // 向下滚动缩小
    else if (scrollY < 0)
    {
        scaleFactor *= 0.9f;
        if (scaleFactor < 1.0f) // 最小1倍
            scaleFactor = 1.0f;
    }

    background->setScale(scaleFactor);
    return true;
}

// 鼠标按下处理（开始拖动）
bool Base::onMouseDown(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        isDragging = true;
        lastMousePos = Vec2(e->getCursorX(), e->getCursorY());
    }
    return true;
}

// 鼠标移动处理（拖动过程）
bool Base::onMouseMove(Event* event)
{
    if (!isDragging || !background) return false;

    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 delta = currentMousePos - lastMousePos;

    // 更新背景位置（正向移动，与鼠标拖动方向一致）
    backgroundPos += delta;
    background->setPosition(backgroundPos);

    lastMousePos = currentMousePos;
    return true;
}

// 鼠标释放处理（结束拖动）
bool Base::onMouseUp(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        isDragging = false;
    }
    return true;
}