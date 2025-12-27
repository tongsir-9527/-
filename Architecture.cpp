#include "Architecture.h"

using namespace cocos2d;

Architecture* Architecture::create(BuildingType type, int level) {
    auto building = new (std::nothrow) Architecture();
    if (building && building->init(type, level)) {
        building->autorelease();
        return building;
    }
    CC_SAFE_DELETE(building);
    return nullptr;
}

bool Architecture::init(BuildingType type, int level) {
    if (!Sprite::init()) {
        return false;
    }

    _type = type;
    _level = level;
    _productionTimer = 0;
    initPropertiesByType();

    std::string imgPath;
    switch (_type) {
        case BuildingType::GOLD_MINE:
            imgPath = "GoldMine.png";
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            imgPath = "ElixirCollector.png";
            break;
        case BuildingType::COMMAND_CENTER:
            imgPath = "CommandCenter.png";
            break;
        case BuildingType::BARRACKS:
            imgPath = "Barracks.png";
            break;
        case BuildingType::ARCHER_TOWER:
            imgPath = "ArcherTower.png";
            break;
        case BuildingType::ELIXIR_FONT:
            imgPath = "ElixirFont.png";
            break;
        case BuildingType::CANNON:
            imgPath = "Cannon.png";
            break;
        case BuildingType::VAULT:
            imgPath = "Vault.png";
            break;
        default:
            imgPath = "UnknownBuilding.png";
    }

    if (!this->initWithFile(imgPath)) {
        return false;
    }

    // 创建等级标签
    _levelLabel = Label::createWithTTF(std::to_string(_level), "fonts/Marker Felt.ttf", 24);
    _levelLabel->setColor(Color3B::YELLOW);
    _levelLabel->setPosition(Vec2(getContentSize().width / 2, getContentSize().height + 20));
    _levelLabel->setVisible(false); // 默认隐藏
    this->addChild(_levelLabel, 2);

    this->schedule(schedule_selector(Architecture::produceResource), 1.0f);
    return true;
}

void Architecture::initPropertiesByType() {
    switch (_type) {
        case BuildingType::COMMAND_CENTER:
            _maxLevel = 10;
            _productionRate = 0;
            _maxHealth = 1000 * _level;
            break;
        case BuildingType::GOLD_MINE:
            _maxLevel = 5;
            _productionRate = 10 * _level;
            _maxHealth = 200 * _level;
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            _maxLevel = 5;
            _productionRate = 8 * _level;
            _maxHealth = 150 * _level;
            break;
        case BuildingType::BARRACKS:
            _maxLevel = 3;
            _productionRate = 0;
            _maxHealth = 300 * _level;
            break;
        case BuildingType::ARCHER_TOWER:
            _maxLevel = 4;
            _productionRate = 0;
            _maxHealth = 400 * _level;
            break;
        case BuildingType::ELIXIR_FONT:
            _maxLevel = 5;
            _productionRate = 0;
            _maxHealth = 150 * _level;
            break;
        case BuildingType::CANNON:
            _maxLevel = 4;
            _productionRate = 0;
            _maxHealth = 350 * _level;
            break;
        case BuildingType::VAULT:
            _maxLevel = 5;
            _productionRate = 0;
            _maxHealth = 250 * _level;
            break;
    }
    _health = _maxHealth; // 初始化血量为最大血量
}

bool Architecture::upgrade() {
    if (_level >= _maxLevel) return false;
    _level++;
    initPropertiesByType(); // 重新初始化属性（包括血量和产量等）
    _levelLabel->setString(std::to_string(_level)); // 更新等级显示
    return true;
}

void Architecture::produceResource(float delta) {
    if (_productionRate <= 0 || !_resourceCallback) return;

    switch (_type) {
        case BuildingType::GOLD_MINE:
            _resourceCallback(ResourceType::GOLD, _productionRate);
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            _resourceCallback(ResourceType::ELIXIR, _productionRate);
            break;
        default:
            break;
    }
}

void Architecture::showLevelLabel() {
    _levelLabel->setVisible(true);
}

void Architecture::hideLevelLabel() {
    _levelLabel->setVisible(false);
}