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

// 修改鼠标滚轮事件处理（反转缩放逻辑）
bool Base::onMouseScroll(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (!background) return false;

    float scrollY = e->getScrollY();

    // 反转原有的缩放逻辑：向上滚动缩小，向下滚动放大
    if (scrollY > 0)
    {
        scaleFactor *= 0.9f;  // 原先是乘以1.1f（放大），现在改为缩小
        if (scaleFactor < 1.0f) // 最小1倍
            scaleFactor = 1.0f;
    }
    else if (scrollY < 0)
    {
        scaleFactor *= 1.1f;  // 原先是乘以0.9f（缩小），现在改为放大
        if (scaleFactor > 3.0f) // 最大3倍
            scaleFactor = 3.0f;
    }

    background->setScale(scaleFactor);

    // 缩放后检查并修正位置，确保背景不超出范围
    constrainBackgroundPosition();

    return true;
}

// 修改鼠标移动事件处理（添加边界限制）
bool Base::onMouseMove(Event* event)
{
    if (!isDragging || !background) return false;

    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 delta = currentMousePos - lastMousePos;

    // 临时计算新位置
    Vec2 newPos = backgroundPos + delta;

    // 保存当前位置，用于约束检查
    backgroundPos = newPos;

    // 约束背景位置，确保不超出游戏界面范围
    constrainBackgroundPosition();

    // 更新背景位置
    background->setPosition(backgroundPos);

    lastMousePos = currentMousePos;
    return true;
}
// 约束背景位置不超出游戏界面范围
void Base::constrainBackgroundPosition()
{
    if (!background) return;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 背景图原始尺寸（base_background.png的实际像素大小）
    const float originalBgWidth = 1824.0f;
    const float originalBgHeight = 1398.0f;

    // 计算缩放后的背景图尺寸
    float scaledBgWidth = originalBgWidth * scaleFactor;
    float scaledBgHeight = originalBgHeight * scaleFactor;

    // 游戏界面的实际可视范围（以origin为起点）
    float gameMinX = origin.x;
    float gameMaxX = origin.x + visibleSize.width;
    float gameMinY = origin.y;
    float gameMaxY = origin.y + visibleSize.height;

    // 计算背景图允许的最大拖动范围（确保背景边缘不超出游戏界面）
    // 背景图锚点为中心，所以边界需要基于中心位置计算
    float minX = gameMinX + scaledBgWidth / 2;   // 左边界（背景中心最大左移位置）
    float maxX = gameMaxX - scaledBgWidth / 2;   // 右边界（背景中心最大右移位置）
    float minY = gameMinY + scaledBgHeight / 2;  // 下边界（背景中心最大下移位置）
    float maxY = gameMaxY - scaledBgHeight / 2;  // 上边界（背景中心最大上移位置）

    // 限制背景图中心位置在计算出的范围内
    backgroundPos.x = clampf(backgroundPos.x, minX, maxX);
    backgroundPos.y = clampf(backgroundPos.y, minY, maxY);

    // 应用限制后的位置
    background->setPosition(backgroundPos);
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