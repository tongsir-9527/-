#ifndef __ARCHITECTURE_H__
#define __ARCHITECTURE_H__

#include "cocos2d.h"

enum class BuildingType {
    COMMAND_CENTER,
    GOLD_MINE,
    ELIXIR_COLLECTOR,
    VAULT,
    BARRACKS,
    WALL,
    ARCHER_TOWER,
    CANNON,
};

enum class ResourceType {
    GOLD,
    ELIXIR,
};
static const int GOLD_MINE_CONSUME = 100;       // 金矿建造消耗（√）
static const int ELIXIR_COLLECTOR_CONSUME = 80; // 圣水收集器建造消耗（√）
static const int BARRACKS_CONSUME = 150;        // 军营建造消耗（√）
static const int WALL_CONSUME = 10;             // 城墙建造消耗（√）
static const int ARCHER_TOWER_CONSUME = 120;    // 弓箭塔建造消耗（√）
static const int CANNON_CONSUME = 100;          // 加农炮建造消耗（√）
static const int VAULT_CONSUME = 100;           // 金库建造消耗（√）
static const int ELIXIR_FONT_CONSUME = 80;      // 圣水罐建造消耗（已找到资源图片）
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

private:
    BuildingType _type;
    int _level;
    int _maxLevel;
    int _productionRate;
    float _productionTimer;
    std::function<void(ResourceType, int)> _resourceCallback;
    void initPropertiesByType();
};

#endif // __ARCHITECTURE_H__