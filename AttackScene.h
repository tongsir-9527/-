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

    // 攻击方建筑
    std::vector<Architecture*> _attackBuildings;

    // 防御方建筑
    struct DefenseBuilding {
        Architecture* building;
        float attackRange;
        float attackDamage;
        float attackInterval;
        float lastAttackTime;
        std::string attackType;
    };
    std::vector<DefenseBuilding> _defenseBuildings;

    // 军队单位
    std::vector<MilitaryUnit*> _militaryUnits;

    // 军队商店相关
    cocos2d::ui::Button* _armyStoreButton;
    cocos2d::Layer* _armyStorePanel;
    cocos2d::ui::ScrollView* _armyScrollView;
    bool _isArmyStoreOpen;
    MilitaryUnit* _draggingUnit;
    cocos2d::Vec2 _unitDragOffset;

    // 资源显示
    int _armyCapacity;
    int _currentArmyCount;
    cocos2d::Label* _armyLabel;

    bool onMouseScroll(cocos2d::Event* event);
    bool onMouseDown(cocos2d::Event* event);
    bool onMouseMove(cocos2d::Event* event);
    bool onMouseUp(cocos2d::Event* event);
    void constrainBackgroundPosition();

    // 初始化攻击方建筑
    void initAttackBuildings();

    // 为建筑添加血条
    void addHealthBarToBuilding(Architecture* building);

    // 更新防御建筑行为
    void updateDefenseBuildings(float delta);

    // 更新防御建筑攻击
    void updateDefenseAttacks(float delta);

    // 军队商店功能
    void initArmyStore();
    void toggleArmyStorePanel();
    void onArmyStoreButtonClicked(cocos2d::Ref* sender);
    void onArmyUnitSelected(cocos2d::Ref* sender);
    void createMilitaryUnit(MilitaryType type);

    // 返回基地按钮
    void menuBackCallback(cocos2d::Ref* pSender);

    // 添加军队显示
    void addArmyDisplay(const cocos2d::Size& visibleSize, const cocos2d::Vec2& origin);
    void updateArmyDisplay();

    // 获取军队类型名称（静态辅助函数）
    static std::string getMilitaryTypeName(MilitaryType type);
};

#endif // __ATTACK_SCENE_H__