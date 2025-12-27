#include "Base.h"
#include "HelloWorldScene.h"
#include "Architecture.h"
#include "Store.h"
#include "MilitaryUnit.h" // 新增军队头文件
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
    _draggingNode = nullptr;

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
        this->addChild(backButton, 5);
    }

    // 注册鼠标事件监听器
    auto listener = EventListenerMouse::create();
    listener->onMouseScroll = CC_CALLBACK_1(Base::onMouseScroll, this);
    listener->onMouseDown = CC_CALLBACK_1(Base::onMouseDown, this);
    listener->onMouseMove = CC_CALLBACK_1(Base::onMouseMove, this);
    listener->onMouseUp = CC_CALLBACK_1(Base::onMouseUp, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 初始化资源
    _gold = 10000000;
    _elixir = 10000000;

    // 初始化指挥中心
    _commandCenter = Architecture::create(BuildingType::COMMAND_CENTER, 1);
    if (_commandCenter) {
        _commandCenter->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        _commandCenter->setScale(scaleFactor);
        this->addChild(_commandCenter, 1);
        _buildings.push_back(_commandCenter);
    }

    // 初始化商店模块
    _store = Store::create(this);
    this->addChild(_store, 5);

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

    // 缩放逻辑
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
    // 缩放所有建筑
    for (auto building : _buildings) {
        building->setScale(scaleFactor);
    }
    // 缩放所有军队单位
    for (auto unit : _militaryUnits) {
        if (unit->isAlive()) {
            unit->setScale(scaleFactor);
        }
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

    if (_draggingNode) {
        // 移动节点（建筑或军队）
        Vec2 newPos = this->convertToNodeSpace(currentMousePos) - _buildingDragOffset;
        _draggingNode->setPosition(newPos);
    }
    else if (background) {
        // 移动背景
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
        Vec2 mousePos = this->convertToNodeSpace(Vec2(e->getCursorX(), e->getCursorY()));

        // 检查是否点击返回按钮
        Node* backButton = getChildByTag(100);
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

        // 检查是否点击军队单位
        for (auto unit : _militaryUnits) {
            if (unit->isAlive() && unit->getBoundingBox().containsPoint(mousePos)) {
                _draggingNode = unit;
                _buildingDragOffset = unit->convertToNodeSpace(mousePos);
                isDragging = true;
                lastMousePos = mousePos;
                return true;
            }
        }

        // 检查是否点击建筑
        for (auto building : _buildings) {
            if (building->getBoundingBox().containsPoint(mousePos)) {
                _draggingNode = building;
                _buildingDragOffset = building->convertToNodeSpace(mousePos);
                isDragging = true;
                lastMousePos = mousePos;
                return true;
            }
        }

        // 开始拖动背景
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
        _draggingNode = nullptr; // 停止拖动
    }
    return true;
}

bool Base::checkCollision(Architecture* newBuilding) {
    Rect newRect = newBuilding->getBoundingBox();
    for (auto existing : _buildings) {
        if (newRect.intersectsRect(existing->getBoundingBox())) {
            return true;
        }
    }
    return false;
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
                errorMsg = "建造失败，黄金资源不足";
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
        case BuildingType::BARRACKS:
            if (_gold >= BARRACKS_CONSUME)
            {
                canBuild = true;
                _gold -= BARRACKS_CONSUME;
            }
            else
            {
                errorMsg = "建造失败，黄金资源不足";
            }
            break;
        case BuildingType::ARCHER_TOWER:
            if (_gold >= ARCHER_TOWER_CONSUME)
            {
                canBuild = true;
                _gold -= ARCHER_TOWER_CONSUME;
            }
            else
            {
                errorMsg = "建造失败，黄金资源不足";
            }
            break;
        case BuildingType::CANNON:
            if (_gold >= CANNON_CONSUME)
            {
                canBuild = true;
                _gold -= CANNON_CONSUME;
            }
            else
            {
                errorMsg = "建造失败，黄金资源不足";
            }
            break;
        case BuildingType::VAULT:
            if (_gold >= VAULT_CONSUME)
            {
                canBuild = true;
                _gold -= VAULT_CONSUME;
            }
            else
            {
                errorMsg = "建造失败，黄金资源不足";
            }
            break;
        case BuildingType::ELIXIR_FONT:
            if (_elixir >= ELIXIR_FONT_CONSUME)
            {
                canBuild = true;
                _elixir -= ELIXIR_FONT_CONSUME;
            }
            else
            {
                errorMsg = "建造失败，圣水资源不足";
            }
            break;
        default:
            errorMsg = "未知建筑类型";
            break;
    }

    if (!canBuild)
    {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        auto label = Label::createWithTTF(errorMsg, "fonts/Marker Felt.ttf", 36);
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
        label->setColor(Color3B::RED);
        this->addChild(label, 100);

        label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
        return;
    }

    // 创建建筑
    Vec2 visibleCenter = Director::getInstance()->getVisibleSize() / 2;
    Architecture* newBuilding = Architecture::create(type, 1);

    if (newBuilding)
    {
        float randomX, randomY;
        bool foundPosition = false;
        for (int i = 0; i < 100; i++) {
            randomX = visibleCenter.x + (rand() % 200 - 100);
            randomY = visibleCenter.y + (rand() % 200 - 100);
            newBuilding->setPosition(Vec2(randomX, randomY));

            if (!checkCollision(newBuilding)) {
                foundPosition = true;
                break;
            }
        }

        if (!foundPosition) {
            // 归还资源
            switch (type)
            {
                case BuildingType::GOLD_MINE: _gold += GOLD_MINE_CONSUME; break;
                case BuildingType::ELIXIR_COLLECTOR: _elixir += ELIXIR_COLLECTOR_CONSUME; break;
                case BuildingType::BARRACKS: _gold += BARRACKS_CONSUME; break;
                case BuildingType::ARCHER_TOWER: _gold += ARCHER_TOWER_CONSUME; break;
                case BuildingType::CANNON: _gold += CANNON_CONSUME; break;
                case BuildingType::VAULT: _gold += VAULT_CONSUME; break;
                case BuildingType::ELIXIR_FONT: _elixir += ELIXIR_FONT_CONSUME; break;
                default: break;
            }

            errorMsg = "没有可用空间建造建筑";
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto origin = Director::getInstance()->getVisibleOrigin();
            auto label = Label::createWithTTF(errorMsg, "fonts/Marker Felt.ttf", 36);
            label->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
            label->setColor(Color3B::RED);
            this->addChild(label, 100);
            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                FadeOut::create(0.5f),
                RemoveSelf::create(),
                nullptr
            ));
            return;
        }

        newBuilding->setScale(scaleFactor);

        // 设置资源回调
        if (type == BuildingType::GOLD_MINE)
        {
            newBuilding->setResourceCallback([this](ResourceType resType, int amount) {
                _gold += amount;
                CCLOG("Gold: %d", _gold);
                });
        }
        else if (type == BuildingType::ELIXIR_COLLECTOR || type == BuildingType::ELIXIR_FONT)
        {
            newBuilding->setResourceCallback([this](ResourceType resType, int amount) {
                _elixir += amount;
                CCLOG("Elixir: %d", _elixir);
                });
        }

        this->addChild(newBuilding, 1);
        _buildings.push_back(newBuilding);
    }
}

// 新增军队创建方法
void Base::createMilitaryUnit(MilitaryType type) {
    bool canCreate = false;
    std::string errorMsg;
    int consume = 0;

    // 检查资源是否足够
    switch (type) {
        case MilitaryType::BOMBER:
            consume = BOMBER_CONSUME;
            if (_elixir >= consume) {
                canCreate = true;
                _elixir -= consume;
            }
            break;
        case MilitaryType::ARCHER:
            consume = ARCHER_CONSUME;
            if (_elixir >= consume) {
                canCreate = true;
                _elixir -= consume;
            }
            break;
        case MilitaryType::BARBARIAN:
            consume = BARBARIAN_CONSUME;
            if (_elixir >= consume) {
                canCreate = true;
                _elixir -= consume;
            }
            break;
        case MilitaryType::GIANT:
            consume = GIANT_CONSUME;
            if (_elixir >= consume) {
                canCreate = true;
                _elixir -= consume;
            }
            break;
    }

    if (!canCreate) {
        errorMsg = "训练失败，圣水资源不足";
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        auto label = Label::createWithTTF(errorMsg, "fonts/Marker Felt.ttf", 36);
        label->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
        label->setColor(Color3B::RED);
        this->addChild(label, 100);
        label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
        return;
    }

    auto baseGround = this->getChildByName<Sprite*>("base_ground");
    if (!baseGround) {
        CCLOG("Error: base_ground node not found!");
        return;
    }

    // 创建军队单位
    auto unit = MilitaryUnit::create(type);
    if (unit) {
        // 将世界坐标转换为base_ground的局部坐标
        Vec2 localPos = baseGround->convertToNodeSpace(worldPos);
        unit->setPosition(localPos);

        // 关键：将军队添加到base_ground作为子节点
        baseGround->addChild(unit);
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