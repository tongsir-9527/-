#include "MouseEvent.h"
#include "Base.h"
#include "Store.h"
#include "HelloWorldScene.h"
#include "AttackScene.h"
#include <algorithm>

USING_NS_CC;

MouseEvent* MouseEvent::create(Base* baseScene)
{
    auto mouseEvent = new (std::nothrow) MouseEvent();
    if (mouseEvent && mouseEvent->init(baseScene))
    {
        mouseEvent->autorelease();
        return mouseEvent;
    }
    CC_SAFE_DELETE(mouseEvent);
    return nullptr;
}

bool MouseEvent::init(Base* baseScene)
{
    if (!Node::init())
    {
        return false;
    }

    _baseScene = baseScene;
    isDragging = false;
    _draggingBuilding = nullptr;
    _selectedBuilding = nullptr;
    _upgradeButton = nullptr;
    _cancelButton = nullptr;
    _pressedButton = nullptr;
    _pressedStoreButton = false;
    background = nullptr;
    scaleFactor = nullptr;
    backgroundPos = nullptr;
    _buildings = nullptr;
    _gold = nullptr;
    _elixir = nullptr;
    _maxGold = nullptr;
    _maxElixir = nullptr;
    _commandCenter = nullptr;
    _store = nullptr;

    return true;
}

bool MouseEvent::onMouseScroll(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (!background || !scaleFactor) return false;

    float scrollY = e->getScrollY();

    if (scrollY > 0)
    {
        *scaleFactor *= 0.9f;
        if (*scaleFactor < 1.0f)
            *scaleFactor = 1.0f;
    }
    else if (scrollY < 0)
    {
        *scaleFactor *= 1.1f;
        if (*scaleFactor > 3.0f)
            *scaleFactor = 3.0f;
    }

    background->setScale(*scaleFactor);
    constrainBackgroundPosition();

    return true;
}

bool MouseEvent::onMouseMove(Event* event)
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
    else if (background && backgroundPos) {
        Vec2 newPos = *backgroundPos + delta;
        *backgroundPos = newPos;
        constrainBackgroundPosition();
        background->setPosition(*backgroundPos);
    }

    lastMousePos = currentMousePos;
    return true;
}

void MouseEvent::constrainBackgroundPosition()
{
    if (!background || !backgroundPos || !scaleFactor) return;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    const float originalBgWidth = 1824.0f;
    const float originalBgHeight = 1398.0f;

    float scaledBgWidth = originalBgWidth * (*scaleFactor);
    float scaledBgHeight = originalBgHeight * (*scaleFactor);

    float gameMinX = origin.x;
    float gameMaxX = origin.x + visibleSize.width;
    float gameMinY = origin.y;
    float gameMaxY = origin.y + visibleSize.height;

    float minX = gameMinX + scaledBgWidth / 2;
    float maxX = gameMaxX - scaledBgWidth / 2;
    float minY = gameMinY + scaledBgHeight / 2;
    float maxY = gameMaxY - scaledBgHeight / 2;

    backgroundPos->x = clampf(backgroundPos->x, minX, maxX);
    backgroundPos->y = clampf(backgroundPos->y, minY, maxY);
    background->setPosition(*backgroundPos);
}

bool MouseEvent::onMouseDown(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 mousePos = _baseScene->convertToNodeSpace(Vec2(e->getCursorX(), e->getCursorY()));

    if (_upgradeButton && _upgradeButton->getBoundingBox().containsPoint(mousePos)) {
        return true; // 防止误触升级按钮
    }

    if (_cancelButton && _cancelButton->getBoundingBox().containsPoint(mousePos)) {
        return true; // 防止误触取消按钮
    }

    // 检查返回按钮
    auto backButton = dynamic_cast<cocos2d::ui::Button*>(_baseScene->getChildByTag(100));
    if (backButton && backButton->getBoundingBox().containsPoint(mousePos)) {
        // 标记按下了返回按钮，在onMouseUp中处理
        _pressedButton = backButton;
        return true;
    }

    // 检查攻击按钮
    auto attackButton = dynamic_cast<cocos2d::ui::Button*>(_baseScene->getChildByTag(101));
    if (attackButton && attackButton->getBoundingBox().containsPoint(mousePos)) {
        // 标记按下了攻击按钮，在onMouseUp中处理
        _pressedButton = attackButton;
        return true;
    }

    // 检查商店按钮
    if (_store && _store->getStoreButton() &&
        _store->getStoreButton()->getBoundingBox().containsPoint(mousePos)) {
        // 商店按钮在mouse up时处理
        _pressedStoreButton = true;
        return true;
    }

    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        // 检查是否点击了建筑
        Vec2 bgMousePos = background->convertToNodeSpace(mousePos);
        for (auto building : *_buildings) {
            if (building->getBoundingBox().containsPoint(bgMousePos)) {
                _draggingBuilding = building;
                _buildingDragOffset = building->convertToNodeSpace(bgMousePos);
                isDragging = true;
                lastMousePos = mousePos;
                hideActionButtons();
                return true;
            }
        }

        isDragging = true;
        lastMousePos = mousePos;
        hideActionButtons();
    }
    else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
    {
        _rightClickStartPos = mousePos;
        Vec2 bgMousePos = background->convertToNodeSpace(mousePos);

        for (auto building : *_buildings)
        {
            if (building->getBoundingBox().containsPoint(bgMousePos))
            {
                showActionButtons(building);
                return true;
            }
        }
        hideActionButtons();
    }
    return true;
}

bool MouseEvent::onMouseUp(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 mousePos = _baseScene->convertToNodeSpace(Vec2(e->getCursorX(), e->getCursorY()));

    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        // 处理按钮点击（在mouse up时）
        // 检查是否点击了返回按钮
        auto backButton = dynamic_cast<cocos2d::ui::Button*>(_baseScene->getChildByTag(100));
        if (backButton && backButton->getBoundingBox().containsPoint(mousePos) &&
            backButton == _pressedButton) {
            menuBackCallback(backButton);
            _pressedButton = nullptr;
            return true;
        }

        // 检查是否点击了攻击按钮
        auto attackButton = dynamic_cast<cocos2d::ui::Button*>(_baseScene->getChildByTag(101));
        if (attackButton && attackButton->getBoundingBox().containsPoint(mousePos) &&
            attackButton == _pressedButton) {
            CCLOG("攻击按钮被点击");
            auto attackScene = AttackScene::createScene();
            if (attackScene)
            {
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, attackScene));
            }
            _pressedButton = nullptr;
            return true;
        }

        // 检查商店按钮
        if (_store && _store->getStoreButton() &&
            _store->getStoreButton()->getBoundingBox().containsPoint(mousePos) &&
            _pressedStoreButton) {
            _store->onStoreButtonClicked(nullptr);
            _pressedStoreButton = false;
            return true;
        }

        // 清空按钮标记
        _pressedButton = nullptr;
        _pressedStoreButton = false;

        isDragging = false;
        _draggingBuilding = nullptr;
    }
    else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
    {
        float distance = mousePos.distance(_rightClickStartPos);
        if (distance < _rightClickThreshold)
        {
            Vec2 bgMousePos = background->convertToNodeSpace(mousePos);
            for (auto building : *_buildings)
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

void MouseEvent::showActionButtons(Architecture* building)
{
    hideActionButtons();
    _selectedBuilding = building;

    building->showLevelLabel();

    _upgradeButton = cocos2d::ui::Button::create("UpgradeButton.png");
    _upgradeButton->setPosition(Vec2(building->getPositionX() + 60, building->getPositionY()));
    _upgradeButton->addClickEventListener([this](Ref* sender) {
        if (_selectedBuilding && isUpgradePossible(_selectedBuilding->getType()))
        {
            _selectedBuilding->upgrade();
            hideActionButtons();
        }
        });
    _baseScene->addChild(_upgradeButton, 20);

    _cancelButton = cocos2d::ui::Button::create("CancelButton.png");
    _cancelButton->setPosition(Vec2(building->getPositionX() - 60, building->getPositionY()));
    _cancelButton->addClickEventListener([this, building](Ref* sender) {
        if (building->getType() == BuildingType::COMMAND_CENTER) {
            auto visibleSize = Director::getInstance()->getVisibleSize();
            auto origin = Director::getInstance()->getVisibleOrigin();

            auto label = Label::createWithTTF("指挥中心不能删除！", "fonts/Marker Felt.ttf", 36);
            label->setPosition(Vec2(origin.x + visibleSize.width / 2,
                origin.y + visibleSize.height / 2));
            label->setColor(Color3B::RED);
            _baseScene->addChild(label, 1000);

            label->runAction(Sequence::create(
                DelayTime::create(2.0f),
                FadeOut::create(0.5f),
                RemoveSelf::create(),
                nullptr
            ));
            return;
        }

        hideActionButtons();

        auto it = std::find(_buildings->begin(), _buildings->end(), building);
        if (it != _buildings->end()) {
            _buildings->erase(it);
        }

        building->runAction(Sequence::create(
            FadeOut::create(0.2f),
            CallFunc::create([building]() {
                building->removeFromParentAndCleanup(true);
                }),
            nullptr
        ));
        });
    _baseScene->addChild(_cancelButton, 20);
}

void MouseEvent::hideActionButtons()
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
    if (_selectedBuilding) {
        _selectedBuilding->hideLevelLabel();
    }
    _selectedBuilding = nullptr;
}

int MouseEvent::calculateUpgradeCost(BuildingType type, int currentLevel)
{
    if (currentLevel >= 3) return 0; // 最大等级

    int baseCost = 0;
    switch (type)
    {
        case BuildingType::GOLD_MINE: baseCost = GOLD_MINE_UPGRADE_BASE; break;
        case BuildingType::ELIXIR_COLLECTOR: baseCost = ELIXIR_COLLECTOR_UPGRADE_BASE; break;
        case BuildingType::BARRACKS: baseCost = BARRACKS_UPGRADE_BASE; break;
        case BuildingType::ARCHER_TOWER: baseCost = ARCHER_TOWER_UPGRADE_BASE; break;
        case BuildingType::CANNON: baseCost = CANNON_UPGRADE_BASE; break;
        case BuildingType::VAULT: baseCost = VAULT_UPGRADE_BASE; break;
        case BuildingType::ELIXIR_FONT: baseCost = ELIXIR_FONT_UPGRADE_BASE; break;
        case BuildingType::COMMAND_CENTER: baseCost = COMMAND_CENTER_UPGRADE_BASE; break;
        default: return 0;
    }

    // 每升一级资源×10
    return baseCost * pow(10, currentLevel - 1);
}

bool MouseEvent::isUpgradePossible(BuildingType type)
{
    if (!_selectedBuilding) return false;

    int currentLevel = _selectedBuilding->getLevel();
    if (currentLevel >= _selectedBuilding->getMaxLevel()) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        auto label = Label::createWithTTF("已达到最大等级", "fonts/Marker Felt.ttf", 36);
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
        label->setColor(Color3B::YELLOW);
        _baseScene->addChild(label, 1000);

        label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
        return false;
    }

    int requiredGold = 0;
    int requiredElixir = 0;

    // 根据建筑类型计算所需资源
    switch (type)
    {
        case BuildingType::GOLD_MINE:
        case BuildingType::BARRACKS:
        case BuildingType::ARCHER_TOWER:
        case BuildingType::CANNON:
        case BuildingType::VAULT:
        case BuildingType::COMMAND_CENTER:
            requiredGold = calculateUpgradeCost(type, currentLevel);
            break;
        case BuildingType::ELIXIR_COLLECTOR:
        case BuildingType::ELIXIR_FONT:
            requiredElixir = calculateUpgradeCost(type, currentLevel);
            break;
        default: return false;
    }

    if (*_gold >= requiredGold && *_elixir >= requiredElixir)
    {
        *_gold -= requiredGold;
        *_elixir -= requiredElixir;
        return true;
    }
    else
    {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        auto label = Label::createWithTTF("升级失败，资源不足", "fonts/Marker Felt.ttf", 36);
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
        label->setColor(Color3B::RED);
        _baseScene->addChild(label, 1000);

        label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
        return false;
    }
}

int MouseEvent::countBuildingsOfType(BuildingType type)
{
    int count = 0;
    for (auto building : *_buildings) {
        if (building->getType() == type) {
            count++;
        }
    }
    return count;
}

bool MouseEvent::canBuildMore(BuildingType type)
{
    // 指挥中心等级决定可建造数量
    int maxCount = _commandCenter ? _commandCenter->getLevel() : 0;
    if (maxCount == 0) return false;

    // 统计当前该类型建筑数量
    int currentCount = countBuildingsOfType(type);

    return currentCount < maxCount;
}

void MouseEvent::menuBackCallback(Ref* pSender)
{
    CCLOG("返回按钮被点击");

    // 保存当前圣水到用户默认值
    if (_elixir) {
        UserDefault::getInstance()->setIntegerForKey("elixir", *_elixir);
        UserDefault::getInstance()->flush();
    }

    auto helloWorldScene = HelloWorld::createScene();
    if (helloWorldScene)
    {
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, helloWorldScene));
    }
}

void MouseEvent::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}