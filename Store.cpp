#include "Store.h"
#include "Base.h"

using namespace cocos2d;
using namespace cocos2d::ui;

Store* Store::create(Base* baseScene)
{
    auto store = new (std::nothrow) Store();
    if (store && store->init(baseScene))
    {
        store->autorelease();
        return store;
    }
    CC_SAFE_DELETE(store);
    return nullptr;
}

bool Store::init(Base* baseScene)
{
    if (!Layer::init())
    {
        return false;
    }

    _baseScene = baseScene;
    _isOpen = false;

    // 初始化商店按钮和面板
    initStoreButton();
    initStorePanel();
    initBuildingScrollContent();

    return true;
}

void Store::initStoreButton()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    _storeButton = Button::create("StoreButton.png");
    if (_storeButton)
    {
        float btnX = origin.x + visibleSize.width - _storeButton->getContentSize().width / 2 - 20;
        float btnY = origin.y + _storeButton->getContentSize().height / 2 + 20;
        _storeButton->setPosition(Vec2(btnX, btnY));
        _storeButton->addClickEventListener(CC_CALLBACK_1(Store::onStoreButtonClicked, this));
        this->addChild(_storeButton, 2);
    }
}

void Store::initStorePanel()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 商店面板容器
    _storePanel = Layer::create();
    _storePanel->setVisible(false);
    this->addChild(_storePanel, 3);

    // 面板高度(屏幕的30%)
    float panelHeight = visibleSize.height * 0.3f;

    // 面板背景
    auto panelBg = LayerColor::create(Color4B(50, 50, 50, 200), visibleSize.width, panelHeight);
    panelBg->setPosition(Vec2(0, 0));
    _storePanel->addChild(panelBg);

    // 遮罩层(点击关闭商店)
    auto mask = LayerColor::create(Color4B(0, 0, 0, 100));
    mask->setContentSize(Size(visibleSize.width, visibleSize.height - panelHeight));
    mask->setPosition(Vec2(0, panelHeight));

    auto maskListener = EventListenerTouchOneByOne::create();
    maskListener->setSwallowTouches(true);
    maskListener->onTouchBegan = [this, mask](Touch* touch, Event* event) {
        Vec2 touchLocation = touch->getLocation();
        Vec2 maskLocation = mask->convertToNodeSpace(touchLocation);
        Rect maskRect = Rect(0, 0, mask->getContentSize().width, mask->getContentSize().height);

        if (maskRect.containsPoint(maskLocation))
        {
            togglePanel();
            return true;
        }
        return false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(maskListener, mask);
    _storePanel->addChild(mask);

    // 建筑滚动视图
    _buildingScrollView = ScrollView::create();
    _buildingScrollView->setContentSize(Size(visibleSize.width, panelHeight));
    _buildingScrollView->setPosition(Vec2(0, 0));
    _buildingScrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _storePanel->addChild(_buildingScrollView);

    // 阻止滚动视图事件穿透
    auto scrollListener = EventListenerTouchOneByOne::create();
    scrollListener->setSwallowTouches(true);
    scrollListener->onTouchBegan = [](Touch* touch, Event* event) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(scrollListener, _buildingScrollView);
}

void Store::initBuildingScrollContent()
{
    const float spacing = 20.0f;
    const float buildingIconSize = 100.0f;
    auto visibleSize = Director::getInstance()->getVisibleSize();

    std::vector<std::pair<BuildingType, std::string>> buildings = {
        {BuildingType::GOLD_MINE, "GoldMine.png"},
        {BuildingType::ELIXIR_COLLECTOR, "ElixirCollector.png"},
        // 可添加更多建筑
    };

    _scrollContentWidth = buildings.size() * (buildingIconSize + spacing) - spacing;

    auto contentLayer = Layer::create();
    contentLayer->setContentSize(Size(_scrollContentWidth, _buildingScrollView->getContentSize().height));
    _buildingScrollView->addChild(contentLayer);
    _buildingScrollView->setInnerContainerSize(contentLayer->getContentSize());

    for (size_t i = 0; i < buildings.size(); ++i)
    {
        auto buildingBtn = Button::create(buildings[i].second);
        buildingBtn->setScale(buildingIconSize / buildingBtn->getContentSize().width);
        buildingBtn->setPosition(Vec2(
            i * (buildingIconSize + spacing) + buildingIconSize / 2,
            _buildingScrollView->getContentSize().height / 2
        ));

        buildingBtn->setTag(static_cast<int>(buildings[i].first));
        buildingBtn->addClickEventListener(CC_CALLBACK_1(Store::onBuildingSelected, this));
        contentLayer->addChild(buildingBtn);
    }
}

void Store::togglePanel()
{
    _isOpen = !_isOpen;
    auto visibleSize = Director::getInstance()->getVisibleSize();

    if (_isOpen)
    {
        _storePanel->setVisible(true);
        _storePanel->setPositionY(-_buildingScrollView->getContentSize().height);
        _storePanel->runAction(MoveTo::create(0.3f, Vec2::ZERO));
    }
    else
    {
        _storePanel->runAction(Sequence::create(
            MoveTo::create(0.3f, Vec2(0, -_buildingScrollView->getContentSize().height)),
            CallFunc::create([this]() {
                _storePanel->setVisible(false);
                }),
            nullptr
        ));
    }
}

void Store::onStoreButtonClicked(Ref* sender)
{
    togglePanel();
}

void Store::onBuildingSelected(Ref* sender)
{
    // 调用Base类的资源检查和建筑创建方法
    // 需要在Base类中添加对应的public方法
    Button* buildingBtn = dynamic_cast<Button*>(sender);
    if (buildingBtn)
    {
        BuildingType type = static_cast<BuildingType>(buildingBtn->getTag());
        // 这里假设Base类有createBuilding方法处理实际创建逻辑
        _baseScene->createBuilding(type);
        togglePanel();
    }
}

bool Store::onScrollTouchBegan(Touch* touch, Event* event)
{
    Vec2 touchLocation = touch->getLocation();
    Vec2 scrollLocation = _buildingScrollView->convertToNodeSpace(touchLocation);
    Rect scrollRect = Rect(0, 0, _buildingScrollView->getContentSize().width, _buildingScrollView->getContentSize().height);

    if (scrollRect.containsPoint(scrollLocation))
    {
        _scrollStartPos = touchLocation;
        return true;
    }
    return false;
}

bool Store::onScrollTouchMoved(Touch* touch, Event* event)
{
    Vec2 currentPos = touch->getLocation();
    Vec2 delta = currentPos - _scrollStartPos;

    Vec2 innerPos = _buildingScrollView->getInnerContainerPosition();
    _buildingScrollView->setInnerContainerPosition(Vec2(innerPos.x + delta.x, innerPos.y));

    _scrollStartPos = currentPos;
    return true;
}

bool Store::onScrollTouchEnded(Touch* touch, Event* event)
{
    return true;
}