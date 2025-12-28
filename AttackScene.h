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

    // 馴僻源膘耟
    std::vector<Architecture*> _attackBuildings;

    // 滅郘源膘耟
    struct DefenseBuilding {
        Architecture* building;
        float attackRange;
        float attackDamage;
        float attackInterval;
        float lastAttackTime;
        std::string attackType;
    };
    std::vector<DefenseBuilding> _defenseBuildings;

    // 濂勦等弇
    std::vector<MilitaryUnit*> _militaryUnits;

    // 濂勦妀虛眈壽
    cocos2d::ui::Button* _armyStoreButton;
    cocos2d::Layer* _armyStorePanel;
    cocos2d::ui::ScrollView* _armyScrollView;
    bool _isArmyStoreOpen;
    MilitaryUnit* _draggingUnit;
    cocos2d::Vec2 _unitDragOffset;

    // 訧埭珆尨
    int _armyCapacity;
    int _currentArmyCount;
    cocos2d::Label* _armyLabel;

    bool onMouseScroll(cocos2d::Event* event);
    bool onMouseDown(cocos2d::Event* event);
    bool onMouseMove(cocos2d::Event* event);
    bool onMouseUp(cocos2d::Event* event);
    void constrainBackgroundPosition();

    // 場宎趙馴僻源膘耟
    void initAttackBuildings();

    // 峈膘耟氝樓悛沭
    void addHealthBarToBuilding(Architecture* building);

    // 載陔滅郘膘耟俴峈
    void updateDefenseBuildings(float delta);
    // 氝樓馴僻毓峓珆尨
    void addAttackRangeToBuilding(Architecture* building);
    // 載陔滅郘膘耟馴僻
    void updateDefenseAttacks(float delta);
    // 載陔濂勦等弇
    void updateMilitaryUnits(float delta);

    // 濂勦妀虛髡夔
    void initArmyStore();
    void toggleArmyStorePanel();
    void onArmyStoreButtonClicked(cocos2d::Ref* sender);
    void onArmyUnitSelected(cocos2d::Ref* sender);
    void createMilitaryUnit(MilitaryType type);

    // 殿隙價華偌聽
    void menuBackCallback(cocos2d::Ref* pSender);

    // 氝樓濂勦珆尨
    void addArmyDisplay(const cocos2d::Size& visibleSize, const cocos2d::Vec2& origin);
    void updateArmyDisplay();

    // 鳳龰濂勦濬倰靡備ㄗ噙怓落翑滲杅ㄘ
    static std::string getMilitaryTypeName(MilitaryType type);
};

#endif // __ATTACK_SCENE_H__