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
static const int GOLD_MINE_CONSUME = 100;
static const int ELIXIR_COLLECTOR_CONSUME = 80;
static const int BARRACKS_CONSUME = 150;
static const int ARCHER_TOWER_CONSUME = 120;
static const int CANNON_CONSUME = 100;
static const int VAULT_CONSUME = 100;
static const int ELIXIR_FONT_CONSUME = 80;

// Éý¼¶ÏûºÄ
static const int GOLD_MINE_UPGRADE_CONSUME = 50;
static const int ELIXIR_COLLECTOR_UPGRADE_CONSUME = 40;
static const int BARRACKS_UPGRADE_CONSUME = 75;
static const int ARCHER_TOWER_UPGRADE_CONSUME = 60;
static const int CANNON_UPGRADE_CONSUME = 50;
static const int VAULT_UPGRADE_CONSUME = 50;
static const int ELIXIR_FONT_UPGRADE_CONSUME = 40;
static const int COMMAND_CENTER_UPGRADE_CONSUME = 200;

class Architecture : public cocos2d::Sprite
{
public:
    static Architecture* create(BuildingType type, int level = 1);
    virtual bool init(BuildingType type, int level);

    BuildingType getType() const { return _type; }
    int getLevel() const { return _level; }
    bool upgrade();
    void produceResource(float delta);
    void setResourceCallback(std::function<void(ResourceType, int)> callback) {
        _resourceCallback = callback;
    }
    int getHealth() const { return _health; }
    int getMaxHealth() const { return _maxHealth; }
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
    cocos2d::Label* _levelLabel;
    void initPropertiesByType();
};

#endif // __ARCHITECTURE_H__