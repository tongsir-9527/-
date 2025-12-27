#ifndef __STORE_H__
#define __STORE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Architecture.h"

class Base; // 前向声明

class Store : public cocos2d::Layer
{
public:
    static Store* create(Base* baseScene);
    virtual bool init(Base* baseScene);

    void togglePanel();
    bool isOpen() const { return _isOpen; }
    cocos2d::ui::Button* getStoreButton() const { return _storeButton; }

    // 将按钮点击事件处理函数改为公有
    void onStoreButtonClicked(Ref* sender);

private:
    Base* _baseScene; // 指向Base场景的指针
    bool _isOpen;
    cocos2d::ui::Button* _storeButton;
    cocos2d::Layer* _storePanel;
    cocos2d::ui::ScrollView* _buildingScrollView;
    cocos2d::Vec2 _scrollStartPos;
    float _scrollContentWidth;

    void initStoreButton();
    void initStorePanel();
    void initBuildingScrollContent();
    void onBuildingSelected(Ref* sender);
    bool onScrollTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
    bool onScrollTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
    bool onScrollTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
};

#endif // __STORE_H__