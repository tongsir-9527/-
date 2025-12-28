#include "MilitaryUnit.h"

using namespace cocos2d;

MilitaryUnit* MilitaryUnit::create(MilitaryType type) {
    auto unit = new (std::nothrow) MilitaryUnit();
    if (unit && unit->init(type)) {
        unit->autorelease();
        return unit;
    }
    CC_SAFE_DELETE(unit);
    return nullptr;
}

bool MilitaryUnit::init(MilitaryType type) {
    if (!Sprite::init()) {
        return false;
    }

    _type = type;
    _attackTimer = 0;
    _attackTarget = nullptr;
    _healthBar = nullptr;
    _isMoving = true;
    _isAttacking = false;
    initPropertiesByType();

    // 设置单位纹理
    std::string imgPath;
    switch (_type) {
        case MilitaryType::BOMBER:
            imgPath = "Bomber.png";
            break;
        case MilitaryType::ARCHER:
            imgPath = "Archer.png";
            break;
        case MilitaryType::BARBARIAN:
            imgPath = "Barbarian.png";
            break;
        case MilitaryType::GIANT:
            imgPath = "Giant.png";
            break;
        default:
            imgPath = "CloseNormal.png"; // 备用纹理
    }

    if (!this->initWithFile(imgPath)) {
        // 如果纹理加载失败，创建一个颜色块作为占位符
        auto placeholder = LayerColor::create(Color4B(100, 100, 200, 255), 50, 50);
        this->addChild(placeholder);
    }

    // 创建血量条背景（红色）
    auto healthBarBg = LayerColor::create(Color4B(255, 0, 0, 255), this->getContentSize().width, 5);
    if (healthBarBg) {
        healthBarBg->setPosition(Vec2(-this->getContentSize().width / 2, this->getContentSize().height / 2 + 10));
        this->addChild(healthBarBg);
    }

    // 创建血量条前景（绿色）
    _healthBar = LayerColor::create(Color4B(0, 255, 0, 255), this->getContentSize().width, 5);
    if (_healthBar) {
        _healthBar->setPosition(Vec2(-this->getContentSize().width / 2, this->getContentSize().height / 2 + 10));
        this->addChild(_healthBar);
    }

    // 启动更新循环
    this->scheduleUpdate();
    return true;
}

void MilitaryUnit::initPropertiesByType() {
    switch (_type) {
        case MilitaryType::BOMBER:
            _maxHealth = 50;
            _attackDamage = 30;
            _attackRange = 50.0f;
            _attackCooldown = 2.0f;
            _moveSpeed = 80.0f; // 移动速度
            break;
        case MilitaryType::ARCHER:
            _maxHealth = 40;
            _attackDamage = 15;
            _attackRange = 90.0f;
            _attackCooldown = 1.5f;
            _moveSpeed = 100.0f;
            break;
        case MilitaryType::BARBARIAN:
            _maxHealth = 80;
            _attackDamage = 20;
            _attackRange = 20.0f;
            _attackCooldown = 1.0f;
            _moveSpeed = 120.0f;
            break;
        case MilitaryType::GIANT:
            _maxHealth = 200;
            _attackDamage = 25;
            _attackRange = 30.0f;
            _attackCooldown = 2.5f;
            _moveSpeed = 60.0f;
            break;
    }
    _health = _maxHealth;
}

Architecture* MilitaryUnit::findNearestBuilding(const std::vector<Architecture*>& buildings) {
    Architecture* nearest = nullptr;
    float minDistance = FLT_MAX;

    for (auto building : buildings) {
        if (building->getHealth() <= 0) continue;

        float distance = this->getPosition().distance(building->getPosition());
        if (distance < minDistance) {
            minDistance = distance;
            nearest = building;
        }
    }

    return nearest;
}

Architecture* MilitaryUnit::findCommandCenter(const std::vector<Architecture*>& buildings) {
    // 先寻找司令部
    for (auto building : buildings) {
        if (building->getHealth() <= 0) continue;

        if (building->getType() == BuildingType::COMMAND_CENTER) {
            return building;
        }
    }

    // 如果没有司令部，则寻找最近的建筑
    return findNearestBuilding(buildings);
}

void MilitaryUnit::update(float delta) {
    if (!isAlive()) return;

    if (_attackTarget && _attackTarget->getHealth() > 0) {
        float distance = _attackTarget->getPosition().distance(this->getPosition());

        if (distance <= _attackRange) {
            // 在攻击范围内，停止移动，开始攻击
            _isMoving = false;
            _isAttacking = true;
            attackTarget(delta);
        }
        else {
            // 不在攻击范围内，继续移动
            _isMoving = true;
            _isAttacking = false;
            moveToTarget(delta);
        }
    }
    else {
        // 没有目标或目标已死亡，停止攻击
        _isAttacking = false;
        _attackTarget = nullptr;
    }
}

void MilitaryUnit::moveToTarget(float delta) {
    if (!_attackTarget || !_isMoving) return;

    Vec2 currentPos = this->getPosition();
    Vec2 targetPos = _attackTarget->getPosition();

    // 计算移动方向
    Vec2 direction = targetPos - currentPos;
    direction.normalize();

    // 计算移动距离
    float moveDistance = _moveSpeed * delta;

    // 移动单位
    this->setPosition(currentPos + direction * moveDistance);
}

void MilitaryUnit::attackTarget(float delta) {
    if (!_attackTarget || !_isAttacking) return;

    _attackTimer += delta;

    if (_attackTimer >= _attackCooldown) {
        // 攻击冷却完成，对建筑造成伤害
        int currentHealth = _attackTarget->getHealth();
        _attackTarget->setHealth(currentHealth - _attackDamage);

        // 重置攻击计时器
        _attackTimer = 0.0f;

        // 显示攻击效果
        CCLOG("Military unit attacks building for %d damage!", _attackDamage);

        // 如果建筑被摧毁
        if (_attackTarget->getHealth() <= 0) {
            _isAttacking = false;
            _attackTarget = nullptr;
        }
    }
}

void MilitaryUnit::takeDamage(int damage) {
    _health -= damage;
    if (_health < 0) _health = 0;

    // 更新血条
    if (_healthBar) {
        float healthPercent = (float)_health / _maxHealth;
        float newWidth = this->getContentSize().width * healthPercent;
        _healthBar->setContentSize(Size(newWidth, 5));
    }

    if (_health <= 0) {
        // 死亡效果
        this->stopAllActions();
        this->unscheduleUpdate();
        this->runAction(Sequence::create(
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
    }
}