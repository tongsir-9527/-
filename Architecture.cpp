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

    // 设置建筑
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
        case BuildingType::BARRACKS:               // 军营
            imgPath = "Barracks.png";
            break;
        case BuildingType::ARCHER_TOWER:           // 弓箭塔
            imgPath = "ArcherTower.png";
            break;
        case BuildingType::ELIXIR_FONT:            // 圣水罐
            imgPath = "ElixirFont.png";
            break;
        case BuildingType::CANNON:                 // 加农炮
            imgPath = "Cannon.png";
            break;
        case BuildingType::VAULT:                  // 金库
            imgPath = "Vault.png";
            break;
        default:
            imgPath = "UnknownBuilding.png";
    }

    if (!this->initWithFile(imgPath)) {
        return false;
    }

    // 启动生产计时器
    this->schedule(schedule_selector(Architecture::produceResource), 1.0f);
    return true;
}
//建筑等级、资源产生等属性初始化
void Architecture::initPropertiesByType() {
    switch (_type) {
        case BuildingType::COMMAND_CENTER:
            _maxLevel = 10;  // 司令部最高10级
            _productionRate = 0;  // 不生产资源
            break;
        case BuildingType::GOLD_MINE:
            _maxLevel = 5;   // 受司令部等级限制
            _productionRate = 10 * _level;  // 每级提升10金币/秒
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            _maxLevel = 5;
            _productionRate = 8 * _level;   // 每级提升8圣水/秒
            break;
        case BuildingType::BARRACKS:
            _maxLevel = 3;
            _productionRate = 0;  // 军营不生产资源
            break;
        case BuildingType::ARCHER_TOWER:
            _maxLevel = 4;
            _productionRate = 0;  // 防御建筑不生产资源
            break;
        case BuildingType::ELIXIR_FONT:
            _maxLevel = 5;
            _productionRate = 0;  // 资源存储建筑不生产资源
            break;
        case BuildingType::CANNON:
            _maxLevel = 4;
            _productionRate = 0;  // 防御建筑不生产资源
            break;
        case BuildingType::VAULT:
            _maxLevel = 5;
            _productionRate = 0;  // 资源存储建筑不生产资源
            break;
    }
}

bool Architecture::upgrade() {
    if (_level >= _maxLevel) return false;
    _level++;
    _productionRate = (_type == BuildingType::GOLD_MINE) ? 10 * _level : 8 * _level;
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