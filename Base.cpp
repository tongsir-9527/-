#include "Base.h"
#include "HelloWorldScene.h"
#include "Architecture.h"
#include "Store.h"
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
    scaleFactor = 1.0f;
    isDragging = false;
    _draggingBuilding = nullptr;

    // 添加背景图
    background = Sprite::create("base_background.png");
    if (background)
    {
        background->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
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
        this->addChild(backButton, 5);  // 提高zOrder确保可点击
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

    // 初始化指挥中心
    _commandCenter = Architecture::create(BuildingType::COMMAND_CENTER, 1);
    if (_commandCenter) {
        _commandCenter->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        _commandCenter->setScale(scaleFactor);  // 设置初始缩放
        this->addChild(_commandCenter, 1);
    }

    // 初始化商店模块(商店包含UI和逻辑)
    _store = Store::create(this);
    this->addChild(_store, 5);  // 提高zOrder确保可点击

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

bool Base::onMouseScroll(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (!background) return false;

    float scrollY = e->getScrollY();

    // 反转缩放逻辑
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
    if (_commandCenter) {
        _commandCenter->setScale(scaleFactor);
    }
    constrainBackgroundPosition();

    return true;
}

bool Base::onMouseMove(Event* event)
{
    if (!isDragging) return false;

    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 delta = currentMousePos - lastMousePos;

    if (_draggingBuilding) {
        // 拖动建筑
        Vec2 newPos = this->convertToNodeSpace(currentMousePos) - _buildingDragOffset;
        _draggingBuilding->setPosition(newPos);
    }
    else if (background) {
        // 拖动背景
        Vec2 newPos = backgroundPos + delta;
        backgroundPos = newPos;
        constrainBackgroundPosition();
        background->setPosition(backgroundPos);
    }

    lastMousePos = currentMousePos;
    return true;
}

void Base::constrainBackgroundPosition()
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

bool Base::onMouseDown(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        // 获取鼠标位置，转换为UI坐标系
        Vec2 mousePos = this->convertToNodeSpace(Vec2(e->getCursorX(), e->getCursorY()));

        // 检查是否点击返回按钮
        Node* backButton = getChildByTag(100); // 假设返回按钮有标签100
        if (backButton && backButton->getBoundingBox().containsPoint(mousePos)) {
            menuBackCallback(backButton);
            return true;
        }

        // 检查是否点击商店按钮
        if (_store && _store->getStoreButton() &&
            _store->getStoreButton()->getBoundingBox().containsPoint(mousePos)) {
            _store->onStoreButtonClicked(nullptr);
            return true;
        }

        // 检查是否点击了大本营
        if (_commandCenter && _commandCenter->getBoundingBox().containsPoint(mousePos)) {
            _draggingBuilding = _commandCenter;
            _buildingDragOffset = _commandCenter->convertToNodeSpace(mousePos);
            isDragging = true;
            lastMousePos = mousePos;
            return true;
        }

        // 如果没有点击任何UI元素，则拖动背景
        isDragging = true;
        lastMousePos = mousePos;
    }
    return true;
}

bool Base::onMouseUp(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        isDragging = false;
        _draggingBuilding = nullptr;  // 停止拖动建筑
    }
    return true;
}


void Base::createBuilding(BuildingType type)
{
    bool canBuild = false;
    std::string errorMsg;

    // 检查资源是否足够
    switch (type)
    {
        case BuildingType::GOLD_MINE:
            if (_gold >= GOLD_MINE_CONSUME)
            {
                canBuild = true;
                _gold -= GOLD_MINE_CONSUME;
            }
            else
            {
                errorMsg = "建造失败，金币资源不足";
            }
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            if (_elixir >= ELIXIR_COLLECTOR_CONSUME)
            {
                canBuild = true;
                _elixir -= ELIXIR_COLLECTOR_CONSUME;
            }
            else
            {
                errorMsg = "建造失败，圣水资源不足";
            }
            break;
        default:
            errorMsg = "未知错误";
            break;
    }

    // 资源不足时显示提示
    if (!canBuild)
    {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        auto label = Label::createWithTTF(errorMsg, "fonts/Marker Felt.ttf", 36);
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
        label->setColor(Color3B::RED);
        this->addChild(label, 100);

        // 2秒后自动移除提示
        label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
        return;
    }

    // 资源足够时创建建筑
    Vec2 visibleCenter = Director::getInstance()->getVisibleSize() / 2;
    Architecture* newBuilding = Architecture::create(type, 1);

    if (newBuilding)
    {
        float randomX = visibleCenter.x + (rand() % 200 - 100);
        float randomY = visibleCenter.y + (rand() % 200 - 100);

        newBuilding->setPosition(Vec2(randomX, randomY));
        newBuilding->setScale(scaleFactor);  // 新建筑也应用当前缩放

        if (type == BuildingType::GOLD_MINE)
        {
            newBuilding->setResourceCallback([this](ResourceType resType, int amount) {
                _gold += amount;
                CCLOG("Gold: %d", _gold);
                });
        }
        else if (type == BuildingType::ELIXIR_COLLECTOR)
        {
            newBuilding->setResourceCallback([this](ResourceType resType, int amount) {
                _elixir += amount;
                CCLOG("Elixir: %d", _elixir);
                });
        }

        this->addChild(newBuilding, 1);
    }
}

void Base::onStoreButtonClicked(Ref* sender)
{
    if (_store) {
        _store->togglePanel();
    }
}

void Base::toggleStorePanel()
{
    // 由Store类内部实现
}

void Base::initBuildingScrollContent()
{
    // 由Store类内部实现
}