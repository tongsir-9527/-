#ifndef __ATTACK_SCENE_H__
#define __ATTACK_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Architecture.h"

class AttackScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();

    CREATE_FUNC(AttackScene);

private:
    cocos2d::Sprite* background;
    float scaleFactor;
    bool isDragging;
    cocos2d::Vec2 lastMousePos;
    cocos2d::Vec2 backgroundPos;

    // 攻击场景的建筑
    std::vector<Architecture*> _attackBuildings;

    // 防御建筑结构体
    struct DefenseBuilding {
        Architecture* building;
        float attackRange;
        float attackDamage;
        float attackInterval;
        float lastAttackTime;
        std::string attackType;
    };
    std::vector<DefenseBuilding> _defenseBuildings;

    bool onMouseScroll(cocos2d::Event* event);
    bool onMouseDown(cocos2d::Event* event);
    bool onMouseMove(cocos2d::Event* event);
    bool onMouseUp(cocos2d::Event* event);
    void constrainBackgroundPosition();

    // 初始化攻击场景建筑
    void initAttackBuildings();

    // 为建筑添加血条
    void addHealthBarToBuilding(Architecture* building);

    // 为防御建筑添加攻击范围显示
    void addAttackRangeToBuilding(Architecture* building);

    // 更新防御建筑逻辑
    void updateDefenseBuildings(float delta);

    // 更新防御建筑攻击
    void updateDefenseAttacks(float delta);
};

#endif // __ATTACK_SCENE_H__