#ifndef __BASE_H__
#define __BASE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Architecture.h"
#include "Store.h"
#include "AttackScene.h"

// 前向声明 MouseEvent 类，避免循环包含
class MouseEvent;

class Base : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();

    void createBuilding(BuildingType type);
    void onStoreButtonClicked(cocos2d::Ref* sender);
    void toggleStorePanel();
    void initBuildingScrollContent();
    void updateResourceDisplays(float delta);

    // 获取指挥中心等级
    int getCommandCenterLevel() const { return _commandCenter ? _commandCenter->getLevel() : 0; }

    // 检查是否可以建造更多的建筑
    bool canBuildMore(BuildingType type);
    // 退出时清除数据
    void onExit() override;
    CREATE_FUNC(Base);

private:
    cocos2d::Sprite* background;
    float scaleFactor;
    cocos2d::Vec2 backgroundPos;
    std::vector<Architecture*> _buildings;
    int _gold;
    int _elixir;
    int _maxGold;     // 最大黄金容量
    int _maxElixir;   // 最大圣水容量

    Architecture* _commandCenter;
    Store* _store;
    MouseEvent* _mouseEvent;  // 使用前向声明的类

    // 资源显示控件
    cocos2d::Label* _goldLabel;
    cocos2d::ui::LoadingBar* _goldBar;
    cocos2d::Label* _elixirLabel;
    cocos2d::ui::LoadingBar* _elixirBar;

    void addResourceDisplays(const cocos2d::Size& visibleSize, const cocos2d::Vec2& origin);
    void createResourceDisplay(const std::string& iconPath, const cocos2d::Color3B& color,
        const cocos2d::Vec2& position, float barWidth, float barHeight, float iconSize);

    // 更新存储容量
    void updateStorageCapacity();

    // 统计建筑数量
    int countBuildingsOfType(BuildingType type);

    // 计算升级成本
    int calculateUpgradeCost(BuildingType type, int currentLevel);
};

#endif // __BASE_H__