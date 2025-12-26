#ifndef __BASE_H__
#define __BASE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Architecture.h"

class Base : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    void menuCloseCallback(cocos2d::Ref* pSender);
    void menuBackCallback(cocos2d::Ref* pSender);

    //鼠标处理方法
    bool onMouseScroll(cocos2d::Event* event);
    bool onMouseDown(cocos2d::Event* event);
    bool onMouseMove(cocos2d::Event* event);
    bool onMouseUp(cocos2d::Event* event);

    // 商店相关方法
    void onStoreButtonClicked(Ref* sender);
    void toggleStorePanel();
    bool onScrollTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    bool onScrollTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    bool onScrollTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
    void onBuildingSelected(Ref* sender);

    CREATE_FUNC(Base);

private:
    cocos2d::Sprite* background;
    float scaleFactor;
    bool isDragging;
    cocos2d::Vec2 lastMousePos;
    cocos2d::Vec2 backgroundPos;
    void constrainBackgroundPosition();

    // 资源数据
    int _gold;
    int _elixir;
    int _darkElixir;

    // 建筑指针
    Architecture* _commandCenter;

    // 商店相关成员
    cocos2d::ui::Button* _storeButton;
    cocos2d::Layer* _storePanel;
    cocos2d::ui::ScrollView* _buildingScrollView;
    bool _isStoreOpen;
    cocos2d::Vec2 _scrollStartPos;
    float _scrollContentWidth;

    void initBuildingScrollContent();
};

#endif // __BASE_H__