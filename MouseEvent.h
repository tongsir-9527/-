#ifndef __MOUSE_EVENT_H__
#define __MOUSE_EVENT_H__

#include "cocos2d.h"
#include "Architecture.h"
#include "Base.h"

class Base; // 前向声明

class MouseEvent : public cocos2d::Node
{
public:
    static MouseEvent* create(Base* baseScene);
    virtual bool init(Base* baseScene);

    bool onMouseScroll(cocos2d::Event* event);
    bool onMouseDown(cocos2d::Event* event);
    bool onMouseMove(cocos2d::Event* event);
    bool onMouseUp(cocos2d::Event* event);

    void constrainBackgroundPosition();
    void showActionButtons(Architecture* building);
    void hideActionButtons();
    bool isUpgradePossible(BuildingType type);

    void setBackground(cocos2d::Sprite* bg) { background = bg; }
    void setBackgroundPos(cocos2d::Vec2& pos) { backgroundPos = &pos; }
    void setScaleFactor(float& factor) { scaleFactor = &factor; }
    void setBuildings(std::vector<Architecture*>* bldgs) { _buildings = bldgs; }
    void setCommandCenter(Architecture* cc) { _commandCenter = cc; }
    void setStore(Store* store) { _store = store; }
    void setGold(int* gold) { _gold = gold; }
    void setElixir(int* elixir) { _elixir = elixir; }
    void setMaxGold(int* maxGold) { _maxGold = maxGold; }
    void setMaxElixir(int* maxElixir) { _maxElixir = maxElixir; }

    void menuBackCallback(cocos2d::Ref* pSender);
    void menuCloseCallback(cocos2d::Ref* pSender);

private:
    Base* _baseScene;
    cocos2d::Sprite* background;
    float* scaleFactor;
    bool isDragging;
    cocos2d::Vec2 lastMousePos;
    cocos2d::Vec2* backgroundPos;
    std::vector<Architecture*>* _buildings;
    int* _gold;
    int* _elixir;
    int* _maxGold;
    int* _maxElixir;

    Architecture* _commandCenter;
    Architecture* _draggingBuilding;
    cocos2d::Vec2 _buildingDragOffset;
    cocos2d::ui::Button* _pressedButton; // 记录按下的按钮
    bool _pressedStoreButton; // 记录是否按下了商店按钮
    Store* _store;
    Architecture* _selectedBuilding;
    cocos2d::ui::Button* _upgradeButton;
    cocos2d::ui::Button* _cancelButton;
    cocos2d::Vec2 _rightClickStartPos;
    float _rightClickThreshold = 5.0f;

    // 升级成本计算
    int calculateUpgradeCost(BuildingType type, int currentLevel);

    // 统计建筑数量
    int countBuildingsOfType(BuildingType type);

    // 检查是否可以建造更多
    bool canBuildMore(BuildingType type);
};

#endif // __MOUSE_EVENT_H__