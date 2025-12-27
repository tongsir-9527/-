#ifndef __ARCHITECTURE_H__
#define __ARCHITECTURE_H__

#include "cocos2d.h"

enum class BuildingType {
    COMMAND_CENTER,
    GOLD_MINE,
    ELIXIR_COLLECTOR,
    VAULT,
    BARRACKS,
    ARCHER_TOWER,
    CANNON,
    ELIXIR_FONT,
};

enum class ResourceType {
    GOLD,
    ELIXIR,
};

// 建筑建造消耗
static const int GOLD_MINE_CONSUME = 100;
static const int ELIXIR_COLLECTOR_CONSUME = 80;
static const int BARRACKS_CONSUME = 150;
static const int ARCHER_TOWER_CONSUME = 120;
static const int CANNON_CONSUME = 120;
static const int VAULT_CONSUME = 130;
static const int ELIXIR_FONT_CONSUME = 130;

// 建筑升级消耗（基础值，每级×10）
static const int GOLD_MINE_UPGRADE_BASE = 50;
static const int ELIXIR_COLLECTOR_UPGRADE_BASE = 40;
static const int BARRACKS_UPGRADE_BASE = 75;
static const int ARCHER_TOWER_UPGRADE_BASE = 60;
static const int CANNON_UPGRADE_BASE = 50;
static const int VAULT_UPGRADE_BASE = 50;
static const int ELIXIR_FONT_UPGRADE_BASE = 40;
static const int COMMAND_CENTER_UPGRADE_BASE = 200;

class Architecture : public cocos2d::Sprite
{
public:
    // 确保这些函数是公开的
    static Architecture* create(BuildingType type, int level = 1);
    virtual bool init(BuildingType type, int level);

    BuildingType getType() const { return _type; }
    int getLevel() const { return _level; }
    int getMaxLevel() const { return _maxLevel; }

    bool upgrade();  // 添加分号
    void produceResource(float delta);

    void setResourceCallback(std::function<void(ResourceType, int)> callback) {
        _resourceCallback = callback;
    }

    int getHealth() const { return _health; }
    int getMaxHealth() const { return _maxHealth; }
    int getDamage() const { return _damage; } // 防御建筑的伤害值

    void showLevelLabel();
    void hideLevelLabel(); 

private:
    BuildingType _type;
    int _level;
    int _maxLevel;
    int _productionRate;
    float _productionTimer;
    std::function<void(ResourceType, int)> _resourceCallback;
    int _health;
    int _maxHealth;
    int _damage; // 防御建筑的伤害值
    cocos2d::Label* _levelLabel;
    void initPropertiesByType();
};

#endif // __ARCHITECTURE_H__