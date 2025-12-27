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

    bool onMouseScroll(cocos2d::Event* event);
    bool onMouseDown(cocos2d::Event* event);
    bool onMouseMove(cocos2d::Event* event);
    bool onMouseUp(cocos2d::Event* event);

    void createBuilding(BuildingType type);
    void onStoreButtonClicked(cocos2d::Ref* sender);
    void toggleStorePanel();
    void initBuildingScrollContent();
    void updateResourceDisplays(float delta);

    // 获取司令部等级
    int getCommandCenterLevel() const { return _commandCenter ? _commandCenter->getLevel() : 0; }

    // 检查是否可以建造更多某种建筑
    bool canBuildMore(BuildingType type);

    CREATE_FUNC(Base);

private:
    cocos2d::Sprite* background;
    float scaleFactor;
    bool isDragging;
    cocos2d::Vec2 lastMousePos;
    cocos2d::Vec2 backgroundPos;
    void constrainBackgroundPosition();
    std::vector<Architecture*> _buildings;
    int _gold;
    int _elixir;
    int _maxGold;     // 最大金币容量
    int _maxElixir;   // 最大圣水容量

    Architecture* _commandCenter;
    Architecture* _draggingBuilding;
    cocos2d::Vec2 _buildingDragOffset;

    cocos2d::ui::Button* _storeButton;
    bool _isStoreOpen;
    cocos2d::Layer* _storePanel;
    cocos2d::ui::ScrollView* _buildingScrollView;

    Store* _store;
    Architecture* _selectedBuilding;
    cocos2d::ui::Button* _upgradeButton;
    cocos2d::ui::Button* _cancelButton;
    cocos2d::Vec2 _rightClickStartPos;
    float _rightClickThreshold = 5.0f;

    void hideActionButtons();
    void showActionButtons(Architecture* building);
    bool isUpgradePossible(BuildingType type);
    void refundResources(BuildingType type, int level);

    // 计算升级所需资源
    int calculateUpgradeCost(BuildingType type, int currentLevel);

    // 更新存储容量
    void updateStorageCapacity();

    // 统计建筑数量
    int countBuildingsOfType(BuildingType type);

    // 资源显示控件
    cocos2d::Label* _goldLabel;
    cocos2d::ui::LoadingBar* _goldBar;
    cocos2d::Label* _elixirLabel;
    cocos2d::ui::LoadingBar* _elixirBar;

    void addResourceDisplays(const cocos2d::Size& visibleSize, const cocos2d::Vec2& origin);
    void createResourceDisplay(const std::string& iconPath, const cocos2d::Color3B& color,
        const cocos2d::Vec2& position, float barWidth, float barHeight, float iconSize);
};

#endif // __BASE_H__