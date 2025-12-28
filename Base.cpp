#include "Base.h"
#include "HelloWorldScene.h"
#include "Architecture.h"
#include "Store.h"
#include <algorithm>
#include "MouseEvent.h"
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

    // 返回按钮（左上角）
    auto backButton = Button::create("BackButton.png");
    if (backButton)
    {
        backButton->setPosition(Vec2(origin.x + backButton->getContentSize().width / 2 + 20,
            origin.y + visibleSize.height - backButton->getContentSize().height / 2 - 20));
        backButton->setTag(100); // 设置tag，用于识别
        backButton->addClickEventListener([this](Ref* sender) {
            CCLOG("返回按钮被点击");
            auto helloWorldScene = HelloWorld::createScene();
            if (helloWorldScene)
            {
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, helloWorldScene));
            }
            });
        this->addChild(backButton, 5);
    }

    // 攻击按钮（左上角，返回按钮下方）
    auto attackButton = Button::create("AttackButton.png");
    if (attackButton)
    {
        // 计算攻击按钮的位置（在返回按钮下方）
        float attackButtonY = origin.y + visibleSize.height - backButton->getContentSize().height - attackButton->getContentSize().height / 2 - 40;
        attackButton->setPosition(Vec2(origin.x + attackButton->getContentSize().width / 2 + 20, attackButtonY));
        attackButton->setTag(101); // 设置tag

        // 修复：使用lambda函数处理点击事件
        attackButton->addClickEventListener([this](Ref* sender) {
            CCLOG("攻击按钮被点击");
            auto attackScene = AttackScene::createScene();
            if (attackScene)
            {
                Director::getInstance()->replaceScene(TransitionFade::create(0.5f, attackScene));
            }
            });

        this->addChild(attackButton, 5);
    }

    _gold = 100;
    _elixir = 80;
    _maxGold = 500;    // 初始最大容量
    _maxElixir = 400;  // 初始最大容量

    _commandCenter = Architecture::create(BuildingType::COMMAND_CENTER, 1);
    if (_commandCenter) {
        _commandCenter->setPosition(Vec2(background->getContentSize().width / 2,
            background->getContentSize().height / 2));
        background->addChild(_commandCenter, 1);
        _buildings.push_back(_commandCenter);
        CCLOG("CommandCenter创建在位置: (%f, %f)",
            _commandCenter->getPositionX(), _commandCenter->getPositionY());
    }

    _store = Store::create(this);
    this->addChild(_store, 5);

    // 创建鼠标事件处理器
    _mouseEvent = MouseEvent::create(this);
    if (_mouseEvent) {
        _mouseEvent->setBackground(background);
        _mouseEvent->setBackgroundPos(backgroundPos);
        _mouseEvent->setScaleFactor(scaleFactor);
        _mouseEvent->setBuildings(&_buildings);
        _mouseEvent->setCommandCenter(_commandCenter);
        _mouseEvent->setStore(_store);
        _mouseEvent->setGold(&_gold);
        _mouseEvent->setElixir(&_elixir);
        _mouseEvent->setMaxGold(&_maxGold);
        _mouseEvent->setMaxElixir(&_maxElixir);

        // 添加鼠标事件监听器
        auto listener = EventListenerMouse::create();
        listener->onMouseScroll = CC_CALLBACK_1(MouseEvent::onMouseScroll, _mouseEvent);
        listener->onMouseDown = CC_CALLBACK_1(MouseEvent::onMouseDown, _mouseEvent);
        listener->onMouseMove = CC_CALLBACK_1(MouseEvent::onMouseMove, _mouseEvent);
        listener->onMouseUp = CC_CALLBACK_1(MouseEvent::onMouseUp, _mouseEvent);
        _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _mouseEvent);

        this->addChild(_mouseEvent);
    }

    // 添加资源显示组件
    addResourceDisplays(visibleSize, origin);

    // 定时更新资源显示
    this->schedule(schedule_selector(Base::updateResourceDisplays), 0.1f);

    return true;
}

void Base::createBuilding(BuildingType type)
{
    // 检查是否达到建造上限
    if (!canBuildMore(type)) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        auto label = Label::createWithTTF("达到建造上限", "fonts/Marker Felt.ttf", 36);
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
        return;
    }

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
                errorMsg = "建造失败，药水资源不足";
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
                errorMsg = "建造失败，药水资源不足";
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
        this->addChild(label, 1000);

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
                if (_gold + amount <= _maxGold) {
                    _gold += amount;
                }
                else {
                    _gold = _maxGold;
                }
                CCLOG("Gold: %d/%d", _gold, _maxGold);
                });
        }
        else if (type == BuildingType::ELIXIR_COLLECTOR || type == BuildingType::ELIXIR_FONT)
        {
            newBuilding->setResourceCallback([this](ResourceType resType, int amount) {
                if (_elixir + amount <= _maxElixir) {
                    _elixir += amount;
                }
                else {
                    _elixir = _maxElixir;
                }
                CCLOG("Elixir: %d/%d", _elixir, _maxElixir);
                });
        }

        background->addChild(newBuilding, 1);
        _buildings.push_back(newBuilding);

        // 如果是金库或药水井，更新存储容量
        if (type == BuildingType::VAULT || type == BuildingType::ELIXIR_FONT) {
            updateStorageCapacity();
        }
    }
}

bool Base::canBuildMore(BuildingType type)
{
    // 指挥中心等级决定可建造数量
    int maxCount = getCommandCenterLevel();
    if (maxCount == 0) return false;

    // 统计当前该类型建筑数量
    int currentCount = countBuildingsOfType(type);

    return currentCount < maxCount;
}

int Base::countBuildingsOfType(BuildingType type)
{
    int count = 0;
    for (auto building : _buildings) {
        if (building->getType() == type) {
            count++;
        }
    }
    return count;
}

void Base::updateStorageCapacity()
{
    // 基础容量
    _maxGold = 500;
    _maxElixir = 400;

    // 计算金库增加的容量
    for (auto building : _buildings) {
        if (building->getType() == BuildingType::VAULT) {
            _maxGold += 500 * pow(10, building->getLevel() - 1);
        }
        else if (building->getType() == BuildingType::ELIXIR_FONT) {
            _maxElixir += 400 * pow(10, building->getLevel() - 1);
        }
    }

    // 确保当前资源不超过最大容量
    if (_gold > _maxGold) _gold = _maxGold;
    if (_elixir > _maxElixir) _elixir = _maxElixir;
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

void Base::addResourceDisplays(const Size& visibleSize, const Vec2& origin)
{
    const float barWidth = 200.0f;
    const float barHeight = 30.0f;
    const float iconSize = 30.0f;
    const float spacing = 10.0f;
    const float topMargin = 20.0f;

    createResourceDisplay(
        "GoldCoin.png",
        Color3B::YELLOW,
        Vec2(visibleSize.width - origin.x - barWidth - iconSize - spacing - 40,
            visibleSize.height - origin.y - topMargin),
        barWidth,
        barHeight,
        iconSize
    );

    createResourceDisplay(
        "Elixir.png",
        Color3B::BLUE,
        Vec2(visibleSize.width - origin.x - barWidth - iconSize - spacing - 40,
            visibleSize.height - origin.y - topMargin - barHeight - spacing),
        barWidth,
        barHeight,
        iconSize
    );
}

void Base::createResourceDisplay(const std::string& iconPath, const Color3B& color,
    const Vec2& position, float barWidth, float barHeight, float iconSize)
{
    auto container = Node::create();
    container->setPosition(position);
    this->addChild(container, 10);

    auto label = Label::createWithTTF("0/0", "fonts/Marker Felt.ttf", 20);
    label->setColor(color);
    label->setAnchorPoint(Vec2(0, 0.5f));
    label->setPosition(Vec2(0, barHeight / 2));
    container->addChild(label);

    float labelWidth = label->getContentSize().width;

    auto barBg = LoadingBar::create("bar_bg.png");
    barBg->setAnchorPoint(Vec2(0, 0.5f));
    barBg->setContentSize(Size(barWidth, barHeight));
    barBg->setPosition(Vec2(labelWidth + 10, barHeight / 2));
    barBg->setPercent(100);
    container->addChild(barBg);

    auto bar = LoadingBar::create("bar_fg.png");
    bar->setAnchorPoint(Vec2(0, 0.5f));
    bar->setContentSize(Size(barWidth, barHeight));
    bar->setPosition(Vec2(labelWidth + 10, barHeight / 2));
    bar->setPercent(50);
    container->addChild(bar);

    auto icon = Sprite::create(iconPath);
    icon->setAnchorPoint(Vec2(0, 0.5f));
    icon->setScale(iconSize / icon->getContentSize().width);
    icon->setPosition(Vec2(labelWidth + 10 + barWidth + 10, barHeight / 2));
    container->addChild(icon);

    if (iconPath == "GoldCoin.png") {
        _goldLabel = label;
        _goldBar = bar;
    }
    else {
        _elixirLabel = label;
        _elixirBar = bar;
    }
}

void Base::updateResourceDisplays(float delta)
{
    if (_goldLabel) _goldLabel->setString(std::to_string(_gold) + "/" + std::to_string(_maxGold));
    if (_goldBar) _goldBar->setPercent(std::min(100.0f, (_gold * 100.0f) / _maxGold));

    if (_elixirLabel) _elixirLabel->setString(std::to_string(_elixir) + "/" + std::to_string(_maxElixir));
    if (_elixirBar) _elixirBar->setPercent(std::min(100.0f, (_elixir * 100.0f) / _maxElixir));
}

int Base::calculateUpgradeCost(BuildingType type, int currentLevel)
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