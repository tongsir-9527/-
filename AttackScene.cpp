#include "AttackScene.h"
#include "Base.h"
#include "HelloWorldScene.h"
#include <algorithm>

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
    _draggingUnit = nullptr;
    _isArmyStoreOpen = false;
    _isPlacingUnit = false;
    _selectedUnitType = MilitaryType::BARBARIAN; // 默认选择野蛮人

    // 初始化军队容量
    _armyCapacity = 50;
    _currentArmyCount = 0;
    _armyLabel = nullptr;
    _placingHintLabel = nullptr;

    // 初始化圣水资源（从用户默认值读取或使用默认值）
    _elixir = cocos2d::UserDefault::getInstance()->getIntegerForKey("elixir", 500);
    _maxElixir = 1000;  // 最大圣水容量

    // 创建背景
    background = Sprite::create("attack_background.png");
    if (!background)
    {
        background = Sprite::create("base_background.png"); // 备用背景
    }

    background->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    backgroundPos = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
    background->setPosition(backgroundPos);
    background->setScale(scaleFactor);
    this->addChild(background, 0);

    // 标题
    auto label = Label::createWithTTF("Attack Mode", "fonts/Marker Felt.ttf", 48);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - label->getContentSize().height));
    this->addChild(label, 1);

    // 返回按钮（左上角）
    auto backButton = Button::create("BackButton.png");
    if (backButton)
    {
        backButton->setPosition(Vec2(origin.x + backButton->getContentSize().width / 2 + 20,
            origin.y + visibleSize.height - backButton->getContentSize().height / 2 - 20));
        backButton->addClickEventListener(CC_CALLBACK_1(AttackScene::menuBackCallback, this));
        this->addChild(backButton, 5);
    }

    // 军队商店按钮（左上角，返回按钮下方）
    _armyStoreButton = Button::create("ArmyStoreButton.png");
    if (!_armyStoreButton)
    {
        _armyStoreButton = Button::create("StoreButton.png"); // 备用按钮
    }

    if (_armyStoreButton)
    {
        float armyButtonY = origin.y + visibleSize.height - backButton->getContentSize().height -
            _armyStoreButton->getContentSize().height / 2 - 40;
        _armyStoreButton->setPosition(Vec2(origin.x + _armyStoreButton->getContentSize().width / 2 + 20, armyButtonY));
        _armyStoreButton->addClickEventListener(CC_CALLBACK_1(AttackScene::onArmyStoreButtonClicked, this));
        this->addChild(_armyStoreButton, 5);
    }

    // 初始化防御建筑
    initAttackBuildings();

    // 初始化军队商店
    initArmyStore();

    // 添加军队数量显示
    addArmyDisplay(visibleSize, origin);

    // 添加圣水显示
    addElixirDisplay(visibleSize, origin);

    // 鼠标事件监听器
    auto listener = EventListenerMouse::create();
    listener->onMouseScroll = CC_CALLBACK_1(AttackScene::onMouseScroll, this);
    listener->onMouseDown = CC_CALLBACK_1(AttackScene::onMouseDown, this);
    listener->onMouseMove = CC_CALLBACK_1(AttackScene::onMouseMove, this);
    listener->onMouseUp = CC_CALLBACK_1(AttackScene::onMouseUp, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 更新循环 - 每秒更新10次
    this->schedule(schedule_selector(AttackScene::updateDefenseBuildings), 0.1f);
    this->schedule(schedule_selector(AttackScene::updateDefenseAttacks), 0.1f);
    this->schedule(schedule_selector(AttackScene::updateMilitaryUnits), 0.1f);

    // 添加放置提示标签
    _placingHintLabel = Label::createWithTTF("Click to place unit", "fonts/Marker Felt.ttf", 24);
    _placingHintLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    _placingHintLabel->setColor(Color3B::YELLOW);
    _placingHintLabel->setVisible(false);
    this->addChild(_placingHintLabel, 10);

    // 每5秒恢复1点圣水（模拟时间流逝）
    this->schedule([this](float dt) {
        if (_elixir < _maxElixir) {
            _elixir += 1;
            updateElixirDisplay();
            cocos2d::UserDefault::getInstance()->setIntegerForKey("elixir", _elixir);
        }
        }, 5.0f, "elixir_recovery");

    return true;
}

bool AttackScene::onMouseScroll(cocos2d::Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);

    float scrollY = e->getScrollY();

    if (scrollY > 0)
    {
        scaleFactor *= 0.9f;
        if (scaleFactor < 0.5f)
            scaleFactor = 0.5f;
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

bool AttackScene::onMouseDown(cocos2d::Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 mousePos = this->convertToNodeSpace(Vec2(e->getCursorX(), e->getCursorY()));

    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        // 检查是否点击了商店按钮
        if (_armyStoreButton && _armyStoreButton->getBoundingBox().containsPoint(mousePos)) {
            // 让按钮的回调处理商店打开/关闭
            return true;
        }

        // 检查是否在放置模式下
        if (_isPlacingUnit) {
            // 检查是否点击了商店面板区域
            if (_armyStorePanel->isVisible()) {
                // 检查是否点击了商店面板内部
                Vec2 storePanelPos = _armyStorePanel->convertToNodeSpace(mousePos);
                Rect storePanelRect = Rect(0, 0, _armyStorePanel->getContentSize().width, _armyStorePanel->getContentSize().height);

                if (storePanelRect.containsPoint(storePanelPos)) {
                    // 点击了商店面板，不放置单位
                    return true;
                }
            }

            // 在鼠标点击位置创建单位
            createMilitaryUnitAtPosition(mousePos, _selectedUnitType);

            // 退出放置模式
            _isPlacingUnit = false;
            _placingHintLabel->setVisible(false);

            return true;
        }

        // 检查是否点击了军队单位进行拖动
        Vec2 bgMousePos = background->convertToNodeSpace(mousePos);
        for (auto unit : _militaryUnits) {
            if (unit->isAlive() && unit->getBoundingBox().containsPoint(bgMousePos)) {
                _draggingUnit = unit;
                _unitDragOffset = unit->convertToNodeSpace(bgMousePos);
                isDragging = true;
                lastMousePos = mousePos;
                return true;
            }
        }

        // 如果没有点击单位，则拖动背景
        isDragging = true;
        lastMousePos = mousePos;
    }

    return true;
}

bool AttackScene::onMouseMove(cocos2d::Event* event)
{
    if (!isDragging) return false;

    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 delta = currentMousePos - lastMousePos;

    if (_draggingUnit) {
        // 拖动军队单位
        Vec2 backgroundPosInScene = background->convertToNodeSpace(currentMousePos);
        Vec2 newPos = backgroundPosInScene - _unitDragOffset;
        _draggingUnit->setPosition(newPos);
    }
    else {
        // 拖动背景
        backgroundPos += delta;
        constrainBackgroundPosition();
        background->setPosition(backgroundPos);
    }

    lastMousePos = currentMousePos;
    return true;
}

bool AttackScene::onMouseUp(cocos2d::Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);

    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        isDragging = false;
        _draggingUnit = nullptr;
    }

    return true;
}

void AttackScene::constrainBackgroundPosition()
{
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

void AttackScene::initAttackBuildings()
{
    // 清空现有建筑
    _attackBuildings.clear();
    _defenseBuildings.clear();

    // 背景中心位置
    Vec2 centerPos = Vec2(background->getContentSize().width / 2,
        background->getContentSize().height / 2);

    // 1. 在中心放置司令部
    Architecture* commandCenter = Architecture::create(BuildingType::COMMAND_CENTER, 1);
    if (commandCenter)
    {
        commandCenter->setPosition(centerPos);
        background->addChild(commandCenter, 1);
        _attackBuildings.push_back(commandCenter);

        // 为司令部添加血条
        addHealthBarToBuilding(commandCenter);
    }

    // 建筑放置的半径和角度间隔
    float radius = 200.0f;
    int numBuildings = 6;
    float angleStep = 360.0f / numBuildings;

    // 建筑类型列表（周围一圈的6种建筑）
    std::vector<BuildingType> buildingTypes = {
        BuildingType::ARCHER_TOWER,    // 弓箭塔
        BuildingType::CANNON,          // 加农炮
        BuildingType::GOLD_MINE,       // 金矿
        BuildingType::ELIXIR_FONT,     // 圣水罐
        BuildingType::VAULT,           // 金库
        BuildingType::ELIXIR_COLLECTOR // 圣水收集器
    };

    // 2. 在周围一圈放置其他建筑
    for (int i = 0; i < numBuildings; ++i)
    {
        float angle = CC_DEGREES_TO_RADIANS(angleStep * i);
        Vec2 buildingPos = centerPos + Vec2(radius * cos(angle), radius * sin(angle));

        Architecture* building = Architecture::create(buildingTypes[i], 1);
        if (building)
        {
            building->setPosition(buildingPos);
            background->addChild(building, 1);
            _attackBuildings.push_back(building);

            // 为建筑添加血条
            addHealthBarToBuilding(building);

            // 为防御建筑添加攻击范围显示和防御属性（仅弓箭塔和加农炮）
            if (buildingTypes[i] == BuildingType::ARCHER_TOWER ||
                buildingTypes[i] == BuildingType::CANNON)
            {
                // 添加攻击范围显示（与绿色对比度强的颜色）
                addAttackRangeToBuilding(building);

                // 初始化防御建筑属性
                DefenseBuilding defense;
                defense.building = building;
                defense.lastAttackTime = 0;

                // 设置防御建筑属性
                if (buildingTypes[i] == BuildingType::ARCHER_TOWER)
                {
                    defense.attackRange = 250.0f;      // 弓箭塔范围大
                    defense.attackDamage = 10.0f;      // 伤害低
                    defense.attackInterval = 1.0f;     // 频率高（每秒攻击）
                    defense.attackType = "ARCHER";
                }
                else if (buildingTypes[i] == BuildingType::CANNON)
                {
                    defense.attackRange = 150.0f;      // 加农炮范围小
                    defense.attackDamage = 30.0f;      // 伤害高
                    defense.attackInterval = 2.0f;     // 频率低（每2秒攻击）
                    defense.attackType = "CANNON";
                }

                _defenseBuildings.push_back(defense);
            }
        }
    }
}

void AttackScene::addHealthBarToBuilding(Architecture* building)
{
    auto healthBarBg = LayerColor::create(Color4B(255, 0, 0, 255), building->getContentSize().width, 5);
    healthBarBg->setPosition(Vec2(-building->getContentSize().width / 2,
        building->getContentSize().height / 2 + 10));
    building->addChild(healthBarBg);

    auto healthBar = LayerColor::create(Color4B(0, 255, 0, 255), building->getContentSize().width, 5);
    healthBar->setPosition(Vec2(-building->getContentSize().width / 2,
        building->getContentSize().height / 2 + 10));
    healthBar->setTag(100); // 用于后续更新血条
    building->addChild(healthBar);
}

void AttackScene::addAttackRangeToBuilding(Architecture* building)
{
    float range = 0.0f;
    Color4F rangeColor;

    if (building->getType() == BuildingType::ARCHER_TOWER)
    {
        range = 400.0f;
        // 使用与绿色对比度强的紫色
        rangeColor = Color4F(0.8f, 0.0f, 0.8f, 0.3f); // 紫色半透明
    }
    else if (building->getType() == BuildingType::CANNON)
    {
        range = 250.0f;
        // 使用与绿色对比度强的橙色
        rangeColor = Color4F(1.0f, 0.5f, 0.0f, 0.3f); // 橙色半透明
    }
    else
    {
        return; // 不是防御建筑，不添加范围显示
    }

    // 创建圆形范围显示
    auto rangeCircle = DrawNode::create();
    rangeCircle->drawCircle(Vec2::ZERO, range, 0, 32, false, 2.0f, 2.0f, rangeColor);
    rangeCircle->setVisible(false); // 默认隐藏，需要时可显示

    // 添加白色边框以提高可见性
    rangeCircle->drawCircle(Vec2::ZERO, range, 0, 32, false, 1.0f, 1.0f, Color4F(1.0f, 1.0f, 1.0f, 0.5f));

    building->addChild(rangeCircle, -1);

    // 添加标签显示攻击范围信息
    auto rangeLabel = Label::createWithTTF(std::to_string(static_cast<int>(range)),
        "fonts/Marker Felt.ttf", 20);
    rangeLabel->setPosition(Vec2(0, -range - 20));
    rangeLabel->setColor(Color3B::WHITE);
    rangeLabel->setVisible(false);
    rangeCircle->addChild(rangeLabel);

    // 存储范围引用（可以使用tag来区分）
    rangeCircle->setTag(999); // 为攻击范围设置特殊tag
}

void AttackScene::updateDefenseBuildings(float delta)
{
    // 更新防御建筑的血条
    for (auto building : _attackBuildings) {
        auto healthBar = dynamic_cast<LayerColor*>(building->getChildByTag(100));
        if (healthBar) {
            float healthPercent = (float)building->getHealth() / building->getMaxHealth();
            healthBar->setContentSize(Size(building->getContentSize().width * healthPercent, 5));

            // 如果建筑血量小于等于0，显示摧毁效果
            if (building->getHealth() <= 0 && building->getOpacity() > 0) {
                building->runAction(FadeOut::create(0.5f));
            }
        }
    }
}

void AttackScene::updateDefenseAttacks(float delta)
{
    static float attackTimer = 0;
    attackTimer += delta;

    if (attackTimer < 0.1f) return; // 每0.1秒检查一次
    attackTimer = 0;

    // 防御建筑攻击军队单位
    for (auto& defense : _defenseBuildings) {
        if (!defense.building || defense.building->getHealth() <= 0) continue;

        // 检查攻击冷却
        defense.lastAttackTime += 0.1f;
        if (defense.lastAttackTime < defense.attackInterval) continue;

        // 寻找攻击范围内的军队单位
        for (auto unit : _militaryUnits) {
            if (unit->isAlive()) {
                float distance = defense.building->getPosition().distance(unit->getPosition());
                if (distance <= defense.attackRange) {
                    // 防御建筑攻击军队单位
                    unit->takeDamage(defense.attackDamage);

                    // 重置攻击计时器
                    defense.lastAttackTime = 0;

                    // 如果单位死亡，更新计数
                    if (!unit->isAlive()) {
                        _currentArmyCount--;
                        updateArmyDisplay();
                    }
                    break; // 每次只攻击一个单位
                }
            }
        }
    }
}

void AttackScene::updateMilitaryUnits(float delta)
{
    // 为每个军队单位分配目标
    for (auto unit : _militaryUnits)
    {
        if (!unit->isAlive()) continue;

        // 如果当前没有目标或目标已死亡，寻找新目标
        if (!unit->getAttackTarget() || unit->getAttackTarget()->getHealth() <= 0)
        {
            Architecture* newTarget = nullptr;

            if (unit->getType() == MilitaryType::GIANT) {
                // 巨人优先攻击司令部
                newTarget = unit->findCommandCenter(_attackBuildings);
            }
            else {
                // 其他单位攻击最近的建筑
                newTarget = unit->findNearestBuilding(_attackBuildings);
            }

            if (newTarget) {
                unit->setAttackTarget(newTarget);
            }
        }
    }

    // 移除死亡的军队单位
    for (auto it = _militaryUnits.begin(); it != _militaryUnits.end();)
    {
        MilitaryUnit* unit = *it;

        if (!unit->isAlive())
        {
            // 单位死亡，从列表中移除并减少计数
            it = _militaryUnits.erase(it);
            _currentArmyCount--;
            updateArmyDisplay();
        }
        else
        {
            ++it;
        }
    }
}

void AttackScene::initArmyStore()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 创建军队商店面板 - 默认隐藏
    _armyStorePanel = Layer::create();
    _armyStorePanel->setVisible(false);
    this->addChild(_armyStorePanel, 10);

    float panelHeight = visibleSize.height * 0.25f;

    auto panelBg = LayerColor::create(Color4B(30, 30, 100, 220), visibleSize.width, panelHeight);
    panelBg->setPosition(Vec2(0, 0));
    _armyStorePanel->addChild(panelBg);

    // 遮罩层，点击关闭面板
    auto mask = LayerColor::create(Color4B(0, 0, 0, 100));
    mask->setContentSize(Size(visibleSize.width, visibleSize.height - panelHeight));
    mask->setPosition(Vec2(0, panelHeight));

    auto maskListener = EventListenerTouchOneByOne::create();
    maskListener->setSwallowTouches(true);
    maskListener->onTouchBegan = [this, mask](Touch* touch, Event* event) {
        Vec2 touchLocation = touch->getLocation();
        Vec2 maskLocation = mask->convertToNodeSpace(touchLocation);
        Rect maskRect = Rect(0, 0, mask->getContentSize().width, mask->getContentSize().height);

        if (maskRect.containsPoint(maskLocation))
        {
            toggleArmyStorePanel();
            return true;
        }
        return false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(maskListener, mask);
    _armyStorePanel->addChild(mask);

    // 创建军队单位滚动视图
    _armyScrollView = ScrollView::create();
    _armyScrollView->setContentSize(Size(visibleSize.width, panelHeight));
    _armyScrollView->setPosition(Vec2(0, 0));
    _armyScrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _armyScrollView->setBounceEnabled(true);
    _armyScrollView->setSwallowTouches(false); // 允许触摸传递到子元素
    _armyStorePanel->addChild(_armyScrollView);

    // 添加军队单位按钮
    const float spacing = 20.0f;
    const float unitIconSize = 80.0f;

    std::vector<std::pair<MilitaryType, std::string>> armyUnits = {
        {MilitaryType::BARBARIAN, "Barbarian.png"},
        {MilitaryType::ARCHER, "Archer.png"},
        {MilitaryType::GIANT, "Giant.png"},
        {MilitaryType::BOMBER, "Bomber.png"}
    };

    float scrollContentWidth = armyUnits.size() * (unitIconSize + spacing) - spacing;

    auto contentLayer = Layer::create();
    contentLayer->setContentSize(Size(scrollContentWidth, _armyScrollView->getContentSize().height));
    _armyScrollView->addChild(contentLayer);
    _armyScrollView->setInnerContainerSize(contentLayer->getContentSize());

    for (size_t i = 0; i < armyUnits.size(); ++i)
    {
        auto unitBtn = Button::create(armyUnits[i].second);
        if (!unitBtn) {
            // 如果图片不存在，使用默认按钮
            unitBtn = Button::create("CloseNormal.png");
            auto label = Label::createWithTTF(getMilitaryTypeName(armyUnits[i].first),
                "fonts/Marker Felt.ttf", 20);
            label->setPosition(Vec2(unitBtn->getContentSize().width / 2,
                unitBtn->getContentSize().height / 2));
            unitBtn->addChild(label);
        }

        unitBtn->setScale(unitIconSize / unitBtn->getContentSize().width);
        unitBtn->setPosition(Vec2(
            i * (unitIconSize + spacing) + unitIconSize / 2,
            _armyScrollView->getContentSize().height / 2
        ));

        unitBtn->setTag(static_cast<int>(armyUnits[i].first));
        unitBtn->addClickEventListener(CC_CALLBACK_1(AttackScene::onArmyUnitSelected, this));

        // 添加单位名称标签
        auto unitName = Label::createWithTTF(getMilitaryTypeName(armyUnits[i].first),
            "fonts/Marker Felt.ttf", 16);
        unitName->setPosition(Vec2(unitBtn->getPositionX(),
            unitBtn->getPositionY() - unitIconSize / 2 - 10));
        unitName->setColor(Color3B::WHITE);
        contentLayer->addChild(unitName);

        // 添加单位消耗标签
        int unitCost = getMilitaryCost(armyUnits[i].first);
        auto unitCostLabel = Label::createWithTTF(std::to_string(unitCost) + " 圣水",
            "fonts/Marker Felt.ttf", 14);
        unitCostLabel->setPosition(Vec2(unitBtn->getPositionX(),
            unitBtn->getPositionY() - unitIconSize / 2 - 30));
        unitCostLabel->setColor(Color3B::BLUE);
        contentLayer->addChild(unitCostLabel);

        contentLayer->addChild(unitBtn);
    }

    // 确保商店面板初始位置正确
    _armyStorePanel->setPosition(Vec2(0, -panelHeight));
}

void AttackScene::toggleArmyStorePanel()
{
    _isArmyStoreOpen = !_isArmyStoreOpen;
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float panelHeight = visibleSize.height * 0.25f;

    if (_isArmyStoreOpen)
    {
        _armyStorePanel->setVisible(true);
        _armyStorePanel->setPositionY(-panelHeight);
        _armyStorePanel->runAction(MoveTo::create(0.3f, Vec2::ZERO));
    }
    else
    {
        _armyStorePanel->runAction(Sequence::create(
            MoveTo::create(0.3f, Vec2(0, -panelHeight)),
            CallFunc::create([this]() {
                _armyStorePanel->setVisible(false);
                }),
            nullptr
        ));
    }
}

void AttackScene::onArmyStoreButtonClicked(cocos2d::Ref* sender)
{
    toggleArmyStorePanel();
}

void AttackScene::onArmyUnitSelected(cocos2d::Ref* sender)
{
    Button* unitBtn = dynamic_cast<Button*>(sender);
    if (unitBtn)
    {
        MilitaryType selectedType = static_cast<MilitaryType>(unitBtn->getTag());

        // 检查军队容量
        if (_currentArmyCount >= _armyCapacity) {
            showMessage("军队容量已满!", Color3B::RED);
            return;
        }

        // 检查圣水是否足够
        int cost = getMilitaryCost(selectedType);
        if (_elixir < cost) {
            showMessage("圣水不足! 需要 " + std::to_string(cost) + " 圣水", Color3B::RED);
            return;
        }

        // 关闭商店面板
        toggleArmyStorePanel();

        // 进入放置模式
        _isPlacingUnit = true;
        _selectedUnitType = selectedType;
        _placingHintLabel->setVisible(true);

        // 显示消耗信息
        std::string hint = "点击放置 " + getMilitaryTypeName(_selectedUnitType) +
            " (消耗 " + std::to_string(cost) + " 圣水)";
        _placingHintLabel->setString(hint);

        CCLOG("Selected unit type: %s, cost %d elixir, waiting for placement",
            getMilitaryTypeName(_selectedUnitType).c_str(), cost);
    }
}

int AttackScene::getMilitaryCost(MilitaryType type) const
{
    switch (type) {
        case MilitaryType::BARBARIAN: return BARBARIAN_COST;
        case MilitaryType::ARCHER: return ARCHER_COST;
        case MilitaryType::GIANT: return GIANT_COST;
        case MilitaryType::BOMBER: return BOMBER_COST;
        default: return 0;
    }
}

void AttackScene::createMilitaryUnitAtPosition(const cocos2d::Vec2& position, MilitaryType type)
{
    // 检查军队容量
    if (_currentArmyCount >= _armyCapacity) {
        showMessage("军队容量已满!", Color3B::RED);
        return;
    }

    // 检查圣水是否足够
    int cost = getMilitaryCost(type);
    if (_elixir < cost) {
        showMessage("圣水不足! 消耗 " + std::to_string(cost) + " 圣水", Color3B::RED);
        return;
    }

    // 扣除圣水
    _elixir -= cost;
    updateElixirDisplay();

    // 保存圣水到用户默认值
    cocos2d::UserDefault::getInstance()->setIntegerForKey("elixir", _elixir);

    // 创建军队单位
    auto unit = MilitaryUnit::create(type);
    if (unit)
    {
        // 将单位放置在鼠标点击位置（相对于背景）
        Vec2 bgPos = background->convertToNodeSpace(position);
        unit->setPosition(bgPos);
        background->addChild(unit, 2);
        _militaryUnits.push_back(unit);

        // 更新军队数量
        _currentArmyCount++;
        updateArmyDisplay();

        // 显示消耗信息
        std::string costMsg = "消耗 " + std::to_string(cost) + " 圣水";
        showMessage(costMsg, Color3B::GREEN);

        CCLOG("Created %s at position (%f, %f), cost %d elixir", getMilitaryTypeName(type).c_str(), bgPos.x, bgPos.y, cost);
    }
}

void AttackScene::showMessage(const std::string& message, const cocos2d::Color3B& color)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto label = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 36);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2));
    label->setColor(color);
    this->addChild(label, 1000);

    label->runAction(Sequence::create(
        DelayTime::create(2.0f),
        FadeOut::create(0.5f),
        RemoveSelf::create(),
        nullptr
    ));
}

void AttackScene::menuBackCallback(cocos2d::Ref* pSender)
{
    // 保存当前圣水到用户默认值
    cocos2d::UserDefault::getInstance()->setIntegerForKey("elixir", _elixir);
    cocos2d::UserDefault::getInstance()->flush();

    auto baseScene = Base::createScene();
    if (baseScene)
    {
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, baseScene));
    }
}

void AttackScene::addArmyDisplay(const Size& visibleSize, const Vec2& origin)
{
    // 创建军队容量显示
    auto container = Node::create();
    container->setPosition(Vec2(origin.x + visibleSize.width - 200,
        origin.y + visibleSize.height - 50));
    this->addChild(container, 10);

    auto armyIcon = Sprite::create("ArmyIcon.png");
    if (!armyIcon) {
        // 如果ArmyIcon.png不存在，尝试创建其他图标
        armyIcon = Sprite::create("CloseNormal.png");
        if (!armyIcon) {
            // 如果CloseNormal.png也不存在，创建一个简单的颜色块
            armyIcon = Sprite::create();
            auto placeholder = LayerColor::create(Color4B(0, 255, 0, 255), 30, 30);
            armyIcon->addChild(placeholder);
        }
        else {
            armyIcon->setScale(0.3f);
        }
    }
    else {
        armyIcon->setScale(0.5f);
    }

    // 确保armyIcon不为nullptr
    if (armyIcon) {
        armyIcon->setPosition(Vec2(0, 0));
        container->addChild(armyIcon);

        _armyLabel = Label::createWithTTF("0/50", "fonts/Marker Felt.ttf", 24);
        _armyLabel->setColor(Color3B::GREEN);
        _armyLabel->setPosition(Vec2(armyIcon->getContentSize().width * 0.5f + 20, 0));
        container->addChild(_armyLabel);
    }
    else {
        // 如果所有创建方法都失败，只创建标签
        _armyLabel = Label::createWithTTF("0/50", "fonts/Marker Felt.ttf", 24);
        _armyLabel->setColor(Color3B::GREEN);
        _armyLabel->setPosition(Vec2(0, 0));
        container->addChild(_armyLabel);
    }
}

void AttackScene::updateArmyDisplay()
{
    if (_armyLabel) {
        _armyLabel->setString(std::to_string(_currentArmyCount) + "/" + std::to_string(_armyCapacity));
    }
}

void AttackScene::addElixirDisplay(const Size& visibleSize, const Vec2& origin)
{
    // 创建圣水显示容器
    auto container = Node::create();
    container->setPosition(Vec2(origin.x + visibleSize.width - 200,
        origin.y + visibleSize.height - 120));  // 放在军队显示下方
    this->addChild(container, 10);

    // 圣水图标
    auto elixirIcon = Sprite::create("Elixir.png");
    if (!elixirIcon) {
        // 备用图标
        elixirIcon = Sprite::create();
        auto placeholder = LayerColor::create(Color4B(0, 0, 255, 255), 30, 30);
        elixirIcon->addChild(placeholder);
    }
    else {
        elixirIcon->setScale(0.5f);
    }

    if (elixirIcon) {
        elixirIcon->setPosition(Vec2(0, 0));
        container->addChild(elixirIcon);
    }

    // 圣水标签
    _elixirLabel = Label::createWithTTF(std::to_string(_elixir) + "/" + std::to_string(_maxElixir),
        "fonts/Marker Felt.ttf", 20);
    _elixirLabel->setColor(Color3B::BLUE);
    _elixirLabel->setPosition(Vec2(30, 0));  // 图标右侧
    container->addChild(_elixirLabel);

    // 圣水进度条背景
    const float barWidth = 150.0f;
    const float barHeight = 15.0f;

    auto barBg = LayerColor::create(Color4B(100, 100, 100, 255), barWidth, barHeight);
    barBg->setPosition(Vec2(0, -20));  // 标签下方
    container->addChild(barBg);

    // 圣水进度条
    _elixirBar = LayerColor::create(Color4B(100, 100, 255, 255), barWidth, barHeight);
    _elixirBar->setPosition(Vec2(0, -20));
    container->addChild(_elixirBar);

    // 初始更新
    updateElixirDisplay();
}

void AttackScene::updateElixirDisplay()
{
    if (_elixirLabel) {
        _elixirLabel->setString(std::to_string(_elixir) + "/" + std::to_string(_maxElixir));
    }

    if (_elixirBar) {
        float percent = static_cast<float>(_elixir) / _maxElixir;
        float newWidth = 150.0f * percent;
        _elixirBar->setContentSize(Size(newWidth, 15));

        // 根据圣水量改变颜色
        if (percent < 0.3f) {
            _elixirBar->setColor(Color3B(255, 0, 0));  // 红色，圣水不足
        }
        else if (percent < 0.6f) {
            _elixirBar->setColor(Color3B(255, 255, 0));  // 黄色，圣水中等
        }
        else {
            _elixirBar->setColor(Color3B(0, 255, 0));  // 绿色，圣水充足
        }
    }
}

std::string AttackScene::getMilitaryTypeName(MilitaryType type)
{
    switch (type) {
        case MilitaryType::BARBARIAN: return "Barbarian";
        case MilitaryType::ARCHER: return "Archer";
        case MilitaryType::GIANT: return "Giant";
        case MilitaryType::BOMBER: return "Bomber";
        default: return "Unknown";
    }
}