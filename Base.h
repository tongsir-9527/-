#ifndef __BASE_H__
#define __BASE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Architecture.h"
#include "Store.h"

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

    // 创建建筑接口(供Store调用)
    void createBuilding(BuildingType type);

    // 商店按钮点击事件
    void onStoreButtonClicked(cocos2d::Ref* sender);
    // 切换商店面板显示状态
    void toggleStorePanel();
    // 初始化建筑滚动内容
    void initBuildingScrollContent();

    CREATE_FUNC(Base);

private:
    cocos2d::Sprite* background;
    float scaleFactor;
    bool isDragging;
    cocos2d::Vec2 lastMousePos;
    cocos2d::Vec2 backgroundPos;
    void constrainBackgroundPosition(); //拖动背景时不能超出边界
    bool checkCollision(Architecture* newBuilding); //建筑之间不能重叠
    std::vector<Architecture*> _buildings;//建筑
    // 资源数据
    int _gold;
    int _elixir;

    // 指挥中心
    Architecture* _commandCenter;

    // 当前拖动的建筑和偏移量
    Architecture* _draggingBuilding;
    cocos2d::Vec2 _buildingDragOffset;

    // 商店相关成员变量
    cocos2d::ui::Button* _storeButton;       // 商店按钮
    bool _isStoreOpen;                       // 商店是否打开
    cocos2d::Layer* _storePanel;             // 商店面板
    cocos2d::ui::ScrollView* _buildingScrollView; // 建筑滚动视图

    // 商店实例
    Store* _store;
};

#endif // __BASE_H__