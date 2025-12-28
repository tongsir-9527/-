#ifndef __ATTACK_SCENE_H__
#define __ATTACK_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Architecture.h"
#include "MilitaryUnit.h"

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

    // 攻击建筑
    std::vector<Architecture*> _attackBuildings;

    // 防御建筑
    struct DefenseBuilding {
        Architecture* building;
        float attackRange;
        float attackDamage;
        float attackInterval;
        float lastAttackTime;
        std::string attackType;
    };
    std::vector<DefenseBuilding> _defenseBuildings;

    // 军事单位
    std::vector<MilitaryUnit*> _militaryUnits;

    // 军队商店相关
    cocos2d::ui::Button* _armyStoreButton;
    cocos2d::Layer* _armyStorePanel;
    cocos2d::ui::ScrollView* _armyScrollView;
    bool _isArmyStoreOpen;
    MilitaryUnit* _draggingUnit;
    cocos2d::Vec2 _unitDragOffset;

    // 军队容量
    int _armyCapacity;
    int _currentArmyCount;
    cocos2d::Label* _armyLabel;

    // 军队放置
    bool _isPlacingUnit;
    MilitaryType _selectedUnitType;
    cocos2d::Label* _placingHintLabel;

    // 圣水资源
    int _elixir;              // 当前圣水资源
    int _maxElixir;           // 最大圣水容量
    cocos2d::Label* _elixirLabel;     // 圣水标签
    cocos2d::LayerColor* _elixirBar; // 圣水进度条（改为LayerColor）

    // 胜利/失败相关
    bool _hasVictory;  // 是否胜利
    bool _hasDefeat;   // 是否失败
    bool _isGameOver;  // 游戏是否结束
    Architecture* _commandCenter; // 指向司令部的指针

    bool onMouseScroll(cocos2d::Event* event);
    bool onMouseDown(cocos2d::Event* event);
    bool onMouseMove(cocos2d::Event* event);
    bool onMouseUp(cocos2d::Event* event);
    void constrainBackgroundPosition();

    // 初始化攻击建筑
    void initAttackBuildings();

    // 为建筑添加血条
    void addHealthBarToBuilding(Architecture* building);

    // 添加防御建筑攻击范围
    void addAttackRangeToBuilding(Architecture* building);

    // 更新防御建筑血量
    void updateDefenseBuildings(float delta);
    // 更新防御建筑攻击
    void updateDefenseAttacks(float delta);
    // 更新军事单位
    void updateMilitaryUnits(float delta);

    // 军队商店初始化
    void initArmyStore();
    void toggleArmyStorePanel();
    void onArmyStoreButtonClicked(cocos2d::Ref* sender);
    void onArmyUnitSelected(cocos2d::Ref* sender);
    void createMilitaryUnitAtPosition(const cocos2d::Vec2& position, MilitaryType type);

    // 返回菜单回调
    void menuBackCallback(cocos2d::Ref* pSender);

    // 添加军队显示
    void addArmyDisplay(const cocos2d::Size& visibleSize, const cocos2d::Vec2& origin);
    void updateArmyDisplay();

    // 添加圣水显示
    void addElixirDisplay(const cocos2d::Size& visibleSize, const cocos2d::Vec2& origin);
    void updateElixirDisplay();

    // 游戏结束检查
    void checkVictoryCondition(float delta);
    void checkDefeatCondition(float delta);
    void showVictoryScene();
    void handleDefeat();

    // 辅助函数和消耗计算
    int getMilitaryCost(MilitaryType type) const;
    void showMessage(const std::string& message, const cocos2d::Color3B& color);

    // 获取军事单位名称（静态方法）
    static std::string getMilitaryTypeName(MilitaryType type);
};

#endif // __ATTACK_SCENE_H__