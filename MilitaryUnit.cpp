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

    // 启动攻击计时器
    this->schedule(schedule_selector(MilitaryUnit::attack), 0.5f);
    return true;
}

void MilitaryUnit::initPropertiesByType() {
    switch (_type) {
        case MilitaryType::BOMBER:
            _maxHealth = 50;
            _attackDamage = 30;
            _attackRange = 150.0f;
            _attackCooldown = 2.0f;
            break;
        case MilitaryType::ARCHER:
            _maxHealth = 40;
            _attackDamage = 15;
            _attackRange = 300.0f;
            _attackCooldown = 1.5f;
            break;
        case MilitaryType::BARBARIAN:
            _maxHealth = 80;
            _attackDamage = 20;
            _attackRange = 100.0f;
            _attackCooldown = 1.0f;
            break;
        case MilitaryType::GIANT:
            _maxHealth = 200;
            _attackDamage = 25;
            _attackRange = 120.0f;
            _attackCooldown = 2.5f;
            break;
    }
    _health = _maxHealth;
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
        this->runAction(Sequence::create(
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
    }
}

void MilitaryUnit::attack(float delta) {
    _attackTimer += delta;
    if (_attackTimer < _attackCooldown || !_attackTarget) return;

    _attackTimer = 0;

    // 检查目标是否有效
    auto targetBuilding = dynamic_cast<Architecture*>(_attackTarget);
    if (targetBuilding && targetBuilding->getHealth() > 0) {
        float distance = targetBuilding->getPosition().distance(this->getPosition());
        if (distance <= _attackRange) {
            // 这里可以添加攻击动画
            // 暂时只记录攻击日志
            CCLOG("Military unit attacks building!");
        }
    }
}