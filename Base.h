#ifndef __BASE_H__
#define __BASE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Architecture.h"
#include "Store.h"
#include "MilitaryUnit.h" // 新增军队头文件

class Base : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    void menuCloseCallback(cocos2d::Ref* pSender);
    void menuBackCallback(cocos2d::Ref* pSender);

    // 鼠标事件
    bool onMouseScroll(cocos2d::Event* event);
    bool onMouseDown(cocos2d::Event* event);
    bool onMouseMove(cocos2d::Event* event);
    bool onMouseUp(cocos2d::Event* event);

    // 建筑相关接口
    void createBuilding(BuildingType type);

    // 新增军队相关接口
    void createMilitaryUnit(MilitaryType type);

    // 商店按钮点击事件
    void onStoreButtonClicked(cocos2d::Ref* sender);
    void toggleStorePanel();
    void initBuildingScrollContent();

    CREATE_FUNC(Base);

private:
    cocos2d::Sprite* background;
    float scaleFactor;
    bool isDragging;
    cocos2d::Vec2 lastMousePos;
    cocos2d::Vec2 backgroundPos;
    void constrainBackgroundPosition();
    bool checkCollision(Architecture* newBuilding);
    std::vector<Architecture*> _buildings; // 建筑列表

    // 新增军队列表
    std::vector<MilitaryUnit*> _militaryUnits;

    // 资源数据
    int _gold;
    int _elixir;

    // 指挥中心
    Architecture* _commandCenter;

    // 拖动相关
    cocos2d::Node* _draggingNode; // 通用拖动节点（建筑/军队）
    cocos2d::Vec2 _buildingDragOffset;

    // 商店相关
    cocos2d::ui::Button* _storeButton;
    bool _isStoreOpen;
    cocos2d::Layer* _storePanel;
    cocos2d::ui::ScrollView* _buildingScrollView;
    Store* _store;
};

#endif // __BASE_H__