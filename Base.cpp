#include "Base.h"
#include "HelloWorldScene.h"
#include "Architecture.h"
#include "Store.h"
#include <algorithm> // 新增：用于std::find
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
    _selectedBuilding = nullptr;
    _upgradeButton = nullptr;
    _cancelButton = nullptr;

    background = Sprite::create("base_background.png");
    if (background)
    {
        background->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        backgroundPos = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
        background->setPosition(backgroundPos);
        background->setScale(scaleFactor);
        this->addChild(background, 0);
    }

    auto label = Label::createWithTTF("Base Scene", "fonts/Marker Felt.ttf", 64);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - label->getContentSize().height));
    this->addChild(label, 1);

    auto backButton = Button::create("BackButton.png");
    if (backButton)
    {
        backButton->setPosition(Vec2(origin.x + backButton->getContentSize().width / 2 + 20,
            origin.y + visibleSize.height - backButton->getContentSize().height / 2 - 20));
        backButton->addClickEventListener(CC_CALLBACK_1(Base::menuBackCallback, this));
        this->addChild(backButton, 5);
    }

    auto listener = EventListenerMouse::create();
    listener->onMouseScroll = CC_CALLBACK_1(Base::onMouseScroll, this);
    listener->onMouseDown = CC_CALLBACK_1(Base::onMouseDown, this);
    listener->onMouseMove = CC_CALLBACK_1(Base::onMouseMove, this);
    listener->onMouseUp = CC_CALLBACK_1(Base::onMouseUp, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    _gold = 10000000;
    _elixir = 10000000;

    _commandCenter = Architecture::create(BuildingType::COMMAND_CENTER, 1);
    if (_commandCenter) {
        _commandCenter->setPosition(Vec2::ZERO);
        background->addChild(_commandCenter, 1);
        _buildings.push_back(_commandCenter);
    }

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

bool Base::onMouseMove(Event* event)
{
    if (!isDragging) return false;

    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 delta = currentMousePos - lastMousePos;

    if (_draggingBuilding) {
        Vec2 backgroundPos = background->convertToNodeSpace(currentMousePos);
        Vec2 newPos = backgroundPos - _buildingDragOffset;
        _draggingBuilding->setPosition(newPos);
    }
    else if (background) {
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
    Vec2 mousePos = this->convertToNodeSpace(Vec2(e->getCursorX(), e->getCursorY()));

    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        Node* backButton = getChildByTag(100);
        if (backButton && backButton->getBoundingBox().containsPoint(mousePos)) {
            menuBackCallback(backButton);
            return true;
        }

        if (_store && _store->getStoreButton() &&
            _store->getStoreButton()->getBoundingBox().containsPoint(mousePos)) {
            _store->onStoreButtonClicked(nullptr);
            return true;
        }

        Vec2 bgMousePos = background->convertToNodeSpace(mousePos);
        for (auto building : _buildings) {
            if (building->getBoundingBox().containsPoint(bgMousePos)) {
                _draggingBuilding = building;
                _buildingDragOffset = building->convertToNodeSpace(bgMousePos);
                isDragging = true;
                lastMousePos = mousePos;
                hideActionButtons();  // 移动时隐藏按钮
                return true;
            }
        }

        isDragging = true;
        lastMousePos = mousePos;
        hideActionButtons();  // 点击空地时隐藏按钮
    }
    // 右键点击逻辑
    else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
    {
        _rightClickStartPos = mousePos;
        Vec2 bgMousePos = background->convertToNodeSpace(mousePos);

        // 检查是否点击了建筑
        for (auto building : _buildings)
        {
            if (building->getBoundingBox().containsPoint(bgMousePos))
            {
                showActionButtons(building);
                return true;
            }
        }
        hideActionButtons();  // 没点到建筑时隐藏按钮
    }
    return true;
}

bool Base::onMouseUp(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 mousePos = this->convertToNodeSpace(Vec2(e->getCursorX(), e->getCursorY()));

    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        isDragging = false;
        _draggingBuilding = nullptr;
    }
    // 右键释放处理
    else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
    {
        // 计算移动距离，判断是否为有效点击
        float distance = mousePos.distance(_rightClickStartPos);
        if (distance < _rightClickThreshold)
        {
            Vec2 bgMousePos = background->convertToNodeSpace(mousePos);
            for (auto building : _buildings)
            {
                if (building->getBoundingBox().containsPoint(bgMousePos))
                {
                    showActionButtons(building);
                    return true;
                }
            }
            hideActionButtons();
        }
    }
    return true;
}

void Base::createBuilding(BuildingType type)
{
    bool canBuild = false;
    std::string errorMsg;

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
        case BuildingType::BARRACKS:
            if (_gold >= BARRACKS_CONSUME)
            {
                canBuild = true;
                _gold -= BARRACKS_CONSUME;
            }
            else
            {
                errorMsg = "建造失败，金币资源不足";
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
                errorMsg = "建造失败，金币资源不足";
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
                errorMsg = "建造失败，金币资源不足";
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
                errorMsg = "建造失败，金币资源不足";
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
        this->addChild(label, 1000); // 提高层级确保显示在最上层

        label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
        return;
    }

    Vec2 visibleCenter = Director::getInstance()->getVisibleSize() / 2;
    Architecture* newBuilding = Architecture::create(type, 1);

    if (newBuilding)
    {
        Vec2 bgPos = background->convertToNodeSpace(visibleCenter);
        newBuilding->setPosition(bgPos);

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

        background->addChild(newBuilding, 1);
        _buildings.push_back(newBuilding);
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
}

void Base::initBuildingScrollContent()
{
}

// 显示建筑操作按钮
void Base::showActionButtons(Architecture* building)
{
    hideActionButtons(); // 移除所有按钮
    _selectedBuilding = building;

    // 显示建筑等级
    building->showLevelLabel();

    // 升级按钮
    _upgradeButton = Button::create("UpgradeButton.png");
    _upgradeButton->setPosition(Vec2(building->getPositionX() + 60, building->getPositionY()));
    _upgradeButton->addClickEventListener([this](Ref* sender) {
        if (_selectedBuilding && isUpgradePossible(_selectedBuilding->getType()))
        {
            _selectedBuilding->upgrade();
            hideActionButtons();
        }
        });
    background->addChild(_upgradeButton, 10);

    // 取消按钮(删除建筑功能)
    _cancelButton = Button::create("CancelButton.png");
    _cancelButton->setPosition(Vec2(building->getPositionX() - 60, building->getPositionY()));
    _cancelButton->addClickEventListener([this, building](Ref* sender) {
        // 从建筑列表中移除
        auto it = std::find(_buildings.begin(), _buildings.end(), building);
        if (it != _buildings.end()) {
            _buildings.erase(it);
        }

        // 从父节点移除并清理
        building->removeFromParentAndCleanup(true);

        hideActionButtons();
        });
    background->addChild(_cancelButton, 10);
}

void Base::hideActionButtons()
{
    if (_upgradeButton)
    {
        _upgradeButton->removeFromParent();
        _upgradeButton = nullptr;
    }
    if (_cancelButton)
    {
        _cancelButton->removeFromParent();
        _cancelButton = nullptr;
    }
    // 隐藏等级标签
    if (_selectedBuilding) {
        _selectedBuilding->hideLevelLabel();
    }
    _selectedBuilding = nullptr;
}

bool Base::isUpgradePossible(BuildingType type)
{
    int requiredGold = 0;
    int requiredElixir = 0;

    // 根据建筑类型获取所需资源
    switch (type)
    {
        case BuildingType::GOLD_MINE: requiredGold = GOLD_MINE_UPGRADE_CONSUME; break;
        case BuildingType::ELIXIR_COLLECTOR: requiredElixir = ELIXIR_COLLECTOR_UPGRADE_CONSUME; break;
        case BuildingType::BARRACKS: requiredGold = BARRACKS_UPGRADE_CONSUME; break;
        case BuildingType::ARCHER_TOWER: requiredGold = ARCHER_TOWER_UPGRADE_CONSUME; break;
        case BuildingType::CANNON: requiredGold = CANNON_UPGRADE_CONSUME; break;
        case BuildingType::VAULT: requiredGold = VAULT_UPGRADE_CONSUME; break;
        case BuildingType::ELIXIR_FONT: requiredElixir = ELIXIR_FONT_UPGRADE_CONSUME; break;
        case BuildingType::COMMAND_CENTER: requiredGold = COMMAND_CENTER_UPGRADE_CONSUME; break;
        default: return false;
    }

    // 检查资源是否足够
    if (_gold >= requiredGold && _elixir >= requiredElixir)
    {
        _gold -= requiredGold;
        _elixir -= requiredElixir;
        return true;
    }
    else
    {
        // 显示升级失败提示
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        auto label = Label::createWithTTF("升级失败，资源不足", "fonts/Marker Felt.ttf", 36);
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
        label->setColor(Color3B::RED);
        this->addChild(label, 1000);

        label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));

        return false;
    }
}

void Base::refundResources(BuildingType type, int level)
{
    // 实现资源退款逻辑
}