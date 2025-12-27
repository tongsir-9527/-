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
    _damage = 0;
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

    // 显示等级标签
    _levelLabel = Label::createWithTTF(std::to_string(_level), "fonts/Marker Felt.ttf", 24);
    _levelLabel->setColor(Color3B::YELLOW);
    _levelLabel->setPosition(Vec2(getContentSize().width / 2, getContentSize().height + 20));
    _levelLabel->setVisible(false);
    this->addChild(_levelLabel, 2);

    this->schedule(schedule_selector(Architecture::produceResource), 1.0f);
    return true;
}

void Architecture::initPropertiesByType() {
    switch (_type) {
        case BuildingType::COMMAND_CENTER:
            _maxLevel = 3; // 所有建筑最多3级
            _productionRate = 0;
            _maxHealth = 1000 * _level; // 基础血量
            break;
        case BuildingType::GOLD_MINE:
            _maxLevel = 3;
            _productionRate = 10 * _level;
            _maxHealth = 200 * (1 << (_level - 1)); // 每级血量×2
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            _maxLevel = 3;
            _productionRate = 8 * _level;
            _maxHealth = 150 * (1 << (_level - 1));
            break;
        case BuildingType::BARRACKS:
            _maxLevel = 3;
            _productionRate = 0;
            _maxHealth = 300 * (1 << (_level - 1));
            break;
        case BuildingType::ARCHER_TOWER:
            _maxLevel = 3;
            _productionRate = 0;
            _maxHealth = 400 * (1 << (_level - 1));
            _damage = 10 * (1 << (_level - 1)); // 基础伤害10，每级×2
            break;
        case BuildingType::ELIXIR_FONT:
            _maxLevel = 3;
            _productionRate = 0;
            _maxHealth = 150 * (1 << (_level - 1));
            break;
        case BuildingType::CANNON:
            _maxLevel = 3;
            _productionRate = 0;
            _maxHealth = 350 * (1 << (_level - 1));
            _damage = 20 * (1 << (_level - 1)); // 基础伤害20，每级×2
            break;
        case BuildingType::VAULT:
            _maxLevel = 3;
            _productionRate = 0;
            _maxHealth = 250 * (1 << (_level - 1));
            break;
    }
    _health = _maxHealth; // 当前血量设为最大血量
}

bool Architecture::upgrade() {
    if (_level >= _maxLevel) return false;
    _level++;
    initPropertiesByType(); // 重新计算属性
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
    if (_levelLabel) {
        _levelLabel->setVisible(true);
    }
}

void Architecture::hideLevelLabel() {
    if (_levelLabel) {
        _levelLabel->setVisible(false);
    }
}