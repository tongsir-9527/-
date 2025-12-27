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
            imgPath = "UnknownUnit.png";
    }

    if (!this->initWithFile(imgPath)) {
        return false;
    }

    // 创建血量条
    auto healthBarBg = LayerColor::create(Color4B(255, 0, 0, 255), getContentSize().width, 5);
    healthBarBg->setPosition(Vec2(-getContentSize().width / 2, getContentSize().height / 2 + 10));
    addChild(healthBarBg);

    // 1. 创建一个100x20的绿色纹理（宽高根据需求调整）
    auto rt = RenderTexture::create(100, 20);
    rt->beginWithClear(0, 255, 0, 255); // RGBA值（绿色不透明）
    rt->end();

    // 2. 用纹理创建Sprite
    auto healthBarSprite = Sprite::createWithTexture(rt->getSprite()->getTexture());

    // 3. 初始化进度条
    _healthBar = ProgressTimer::create(healthBarSprite);    
    _healthBar->setType(ProgressTimer::Type::BAR);
    _healthBar->setMidpoint(Vec2(0, 0.5f));
    _healthBar->setBarChangeRate(Vec2(1, 0));
    _healthBar->setPercentage(100);
    _healthBar->setContentSize(healthBarBg->getContentSize());
    _healthBar->setPosition(healthBarBg->getContentSize() / 2);
    healthBarBg->addChild(_healthBar);

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
    _healthBar->setPercentage((float)_health / _maxHealth * 100);

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
    // 简单攻击逻辑（实际项目中可添加伤害判定）
    if (_attackTarget->getPosition().distance(getPosition()) <= _attackRange) {
        // 这里可以添加攻击动画和伤害处理
    }
}