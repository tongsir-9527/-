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

// 新增军队类型枚举
enum class MilitaryType {
    BOMBER,       // 炸弹人
    ARCHER,       // 弓箭手
    BARBARIAN,    // 野蛮人
    GIANT         // 巨人
};

enum class ResourceType {
    GOLD,
    ELIXIR,
};

// 建筑消耗定义
static const int GOLD_MINE_CONSUME = 100;       // 金矿消耗
static const int ELIXIR_COLLECTOR_CONSUME = 80; // 圣水收集器消耗
static const int BARRACKS_CONSUME = 150;        // 兵营消耗
static const int ARCHER_TOWER_CONSUME = 120;    // 弓箭塔消耗
static const int CANNON_CONSUME = 100;          // 加农炮消耗
static const int VAULT_CONSUME = 100;           // 金库消耗
static const int ELIXIR_FONT_CONSUME = 80;      // 圣水瓶消耗

// 军队消耗定义
static const int BOMBER_CONSUME = 50;        // 炸弹人消耗（圣水）
static const int ARCHER_CONSUME = 30;        // 弓箭手消耗（圣水）
static const int BARBARIAN_CONSUME = 40;     // 野蛮人消耗（圣水）
static const int GIANT_CONSUME = 100;        // 巨人消耗（圣水）

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