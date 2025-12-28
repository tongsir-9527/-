#ifndef __MILITARY_UNIT_H__
#define __MILITARY_UNIT_H__

#include "cocos2d.h"
#include "Architecture.h"

// 军队类型枚举
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

    MilitaryType getType() const { return _type; }
    int getHealth() const { return _health; }
    int getMaxHealth() const { return _maxHealth; }
    bool isAlive() const { return _health > 0; }
    void takeDamage(int damage);
    void attack(float delta);
    void setAttackTarget(cocos2d::Node* target) { _attackTarget = target; }

private:
    MilitaryType _type;
    int _health;
    int _maxHealth;
    int _attackDamage;
    float _attackRange;
    float _attackCooldown;
    float _attackTimer;
    cocos2d::Node* _attackTarget;

    void initPropertiesByType();
    cocos2d::LayerColor* _healthBar;
};

#endif // __MILITARY_UNIT_H__