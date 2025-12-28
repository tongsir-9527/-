#ifndef __MILITARY_UNIT_H__
#define __MILITARY_UNIT_H__

#include "cocos2d.h"
#include "Architecture.h"

// 濂勦濬倰繹撼
enum class MilitaryType {
    BOMBER,
    ARCHER,
    BARBARIAN,
    GIANT,
};

class MilitaryUnit : public cocos2d::Sprite
{
public:
    static MilitaryUnit* create(MilitaryType type);
    virtual bool init(MilitaryType type);

    void update(float delta) override;

    MilitaryType getType() const { return _type; }
    int getHealth() const { return _health; }
    int getMaxHealth() const { return _maxHealth; }
    bool isAlive() const { return _health > 0; }
    void takeDamage(int damage);
    void setAttackTarget(Architecture* target) { _attackTarget = target; }
    Architecture* getAttackTarget() const { return _attackTarget; }

    // 寻找最近的建筑作为目标
    Architecture* findNearestBuilding(const std::vector<Architecture*>& buildings);

    // 巨人寻找司令部作为目标
    Architecture* findCommandCenter(const std::vector<Architecture*>& buildings);

    // 移动到目标
    void moveToTarget(float delta);

    // 攻击目标
    void attackTarget(float delta);

private:
    MilitaryType _type;
    int _health;
    int _maxHealth;
    int _attackDamage;
    float _attackRange;
    float _attackCooldown;
    float _attackTimer;
    float _moveSpeed;
    Architecture* _attackTarget;
    bool _isMoving;
    bool _isAttacking;

    void initPropertiesByType();
    cocos2d::LayerColor* _healthBar;
};

#endif // __MILITARY_UNIT_H__