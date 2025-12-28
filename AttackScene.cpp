#include "AttackScene.h"
#include "Base.h"
#include "HelloWorldScene.h"
#include "Architecture.h"
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

    // 初始化攻击场景的建筑
    initAttackBuildings();

    // 鼠标事件监听器（用于地图拖动和缩放）
    auto listener = EventListenerMouse::create();
    listener->onMouseScroll = CC_CALLBACK_1(AttackScene::onMouseScroll, this);
    listener->onMouseDown = CC_CALLBACK_1(AttackScene::onMouseDown, this);
    listener->onMouseMove = CC_CALLBACK_1(AttackScene::onMouseMove, this);
    listener->onMouseUp = CC_CALLBACK_1(AttackScene::onMouseUp, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 定时器用于防御建筑攻击
    this->schedule(schedule_selector(AttackScene::updateDefenseBuildings), 0.1f);

    return true;
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

    // 建筑类型列表
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

            // 为防御建筑添加攻击范围显示（仅弓箭塔和加农炮）
            if (buildingTypes[i] == BuildingType::ARCHER_TOWER ||
                buildingTypes[i] == BuildingType::CANNON)
            {
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
    // 创建血条背景
    auto healthBarBg = Sprite::create("bar_bg.png");
    if (!healthBarBg) {
        // 如果bar_bg.png不存在，创建一个红色矩形作为血条背景
        healthBarBg = Sprite::create();
        healthBarBg->setColor(Color3B::BLACK);
    }
    healthBarBg->setScaleX(0.8f);
    healthBarBg->setScaleY(0.3f);
    healthBarBg->setPosition(Vec2(0, building->getContentSize().height / 2 + 15));
    building->addChild(healthBarBg, 10);

    // 创建血条前景
    auto healthBar = ui::LoadingBar::create("bar_fg.png");
    if (!healthBar) {
        // 如果bar_fg.png不存在，使用默认颜色
        healthBar = ui::LoadingBar::create();
        healthBar->setColor(Color3B::GREEN);
    }
    healthBar->setScaleX(0.8f);
    healthBar->setScaleY(0.3f);
    healthBar->setPosition(Vec2(0, building->getContentSize().height / 2 + 15));
    healthBar->setPercent(100.0f);
    healthBar->setDirection(ui::LoadingBar::Direction::LEFT);
    building->addChild(healthBar, 11);

    // 存储血条引用
    building->setUserData(healthBar);

    // 存储建筑当前血量信息（使用自定义标签）
    building->setTag(1000 + building->getHealth()); // 使用tag存储血量信息
}

void AttackScene::addAttackRangeToBuilding(Architecture* building)
{
    // 创建攻击范围显示（半透明圆圈）
    float range = 0.0f;
    Color4F rangeColor;

    if (building->getType() == BuildingType::ARCHER_TOWER)
    {
        range = 250.0f;
        rangeColor = Color4F(0, 1, 0, 0.2f); // 绿色半透明
    }
    else if (building->getType() == BuildingType::CANNON)
    {
        range = 150.0f;
        rangeColor = Color4F(1, 0, 0, 0.2f); // 红色半透明
    }
    else
    {
        return; // 不是防御建筑，不添加范围显示
    }

    // 创建圆形范围显示
    auto rangeCircle = DrawNode::create();
    rangeCircle->drawCircle(Vec2::ZERO, range, 0, 32, false, 1.0f, 1.0f, rangeColor);
    rangeCircle->setVisible(false); // 默认隐藏，需要时可显示
    building->addChild(rangeCircle, -1);

    // 存储范围引用到building的userData中
    // 注意：userData只能存一个指针，我们可以使用自定义属性
    building->setUserData(rangeCircle);
}

void AttackScene::updateDefenseBuildings(float delta)
{
    // 这里未来会添加军队识别和攻击逻辑
    // 目前先更新血条显示

    for (auto& building : _attackBuildings)
    {
        // 获取血条引用
        ui::LoadingBar* healthBar = static_cast<ui::LoadingBar*>(building->getUserData());
        if (healthBar)
        {
            float currentHealth = static_cast<float>(building->getHealth());
            float maxHealth = static_cast<float>(building->getMaxHealth());
            float percent = (currentHealth / maxHealth) * 100.0f;
            healthBar->setPercent(percent);

            // 根据血量改变颜色
            if (percent > 70)
                healthBar->setColor(Color3B::GREEN);
            else if (percent > 30)
                healthBar->setColor(Color3B::YELLOW);
            else
                healthBar->setColor(Color3B::RED);
        }
    }

    // 更新防御建筑攻击逻辑（未来添加军队后会完善）
    updateDefenseAttacks(delta);
}

void AttackScene::updateDefenseAttacks(float delta)
{
    // 暂时只更新计时，未来会添加攻击逻辑
    for (auto& defense : _defenseBuildings)
    {
        defense.lastAttackTime += delta;

        // 这里未来会添加检测范围内敌人的逻辑
        // 如果检测到敌人且冷却时间结束，则进行攻击

        // 伪代码示例：
        /*
        if (defense.lastAttackTime >= defense.attackInterval)
        {
            // 检测范围内的敌人
            auto enemyInRange = findEnemyInRange(defense.building->getPosition(), defense.attackRange);
            if (enemyInRange)
            {
                // 进行攻击
                attackEnemy(enemyInRange, defense.attackDamage);
                defense.lastAttackTime = 0;

                // 显示攻击效果
                showAttackEffect(defense.building, enemyInRange);
            }
        }
        */
    }
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

        // 检查是否点击了建筑（用于测试）
        Vec2 bgMousePos = background->convertToNodeSpace(mousePos);
        for (auto building : _attackBuildings)
        {
            if (building->getBoundingBox().containsPoint(bgMousePos))
            {
                CCLOG("点击了建筑: %d, 血量: %d/%d",
                    (int)building->getType(),
                    building->getHealth(),
                    building->getMaxHealth());

                // 显示/隐藏攻击范围（仅限防御建筑）
                if (building->getType() == BuildingType::ARCHER_TOWER ||
                    building->getType() == BuildingType::CANNON)
                {
                    // 搜索子节点中的DrawNode
                    for (auto child : building->getChildren())
                    {
                        if (dynamic_cast<DrawNode*>(child))
                        {
                            bool isVisible = child->isVisible();
                            child->setVisible(!isVisible);
                            break;
                        }
                    }
                }
                return true;
            }
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