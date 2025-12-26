#include "Base.h"
#include "HelloWorldScene.h"
#include "Architecture.h"
USING_NS_CC;
using namespace cocos2d::ui;

Scene* Base::createScene()
{
    return Base::create();
}

bool Base::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    scaleFactor = 1.0f;
    isDragging = false;

    // 添加背景图
    background = Sprite::create("base_background.png");
    if (background)
    {
        background->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        backgroundPos = Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
        background->setPosition(backgroundPos);
        background->setScale(scaleFactor);
        this->addChild(background, 0);
    }

    // 添加标题
    auto label = Label::createWithTTF("Base Scene", "fonts/Marker Felt.ttf", 64);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height - label->getContentSize().height));
    this->addChild(label, 1);

    // 添加返回按钮（提高层级避免被遮挡）
    auto backButton = Button::create("BackButton.png");
    if (backButton)
    {
        backButton->setPosition(Vec2(origin.x + backButton->getContentSize().width / 2 + 20,
            origin.y + visibleSize.height - backButton->getContentSize().height / 2 - 20));
        backButton->addClickEventListener(CC_CALLBACK_1(Base::menuBackCallback, this));
        this->addChild(backButton, 4); // 提高层级到4，确保在遮罩上方
    }

    // 添加关闭按钮
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(Base::menuCloseCallback, this));
    closeItem->setTag(999);
    if (closeItem)
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));

        auto menu = Menu::create(closeItem, NULL);
        menu->setPosition(Vec2::ZERO);
        this->addChild(menu, 1);
    }

    // 注册鼠标事件监听器
    auto listener = EventListenerMouse::create();
    listener->onMouseScroll = CC_CALLBACK_1(Base::onMouseScroll, this);
    listener->onMouseDown = CC_CALLBACK_1(Base::onMouseDown, this);
    listener->onMouseMove = CC_CALLBACK_1(Base::onMouseMove, this);
    listener->onMouseUp = CC_CALLBACK_1(Base::onMouseUp, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 初始化资源
    _gold = 0;
    _elixir = 0;
    _darkElixir = 0;

    // 初始化指挥中心
    _commandCenter = Architecture::create(BuildingType::COMMAND_CENTER, 1);
    if (_commandCenter) {
        _commandCenter->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        this->addChild(_commandCenter, 1);
    }

    // 添加商店按钮
    _storeButton = Button::create("StoreButton.png");
    if (_storeButton) {
        float btnX = origin.x + visibleSize.width - _storeButton->getContentSize().width / 2 - 20;
        float btnY = origin.y + _storeButton->getContentSize().height / 2 + 20;
        _storeButton->setPosition(Vec2(btnX, btnY));
        _storeButton->addClickEventListener(CC_CALLBACK_1(Base::onStoreButtonClicked, this));
        this->addChild(_storeButton, 2);
    }

    // 初始化商店面板(默认隐藏)
    _isStoreOpen = false;
    _storePanel = Layer::create();
    _storePanel->setVisible(false);
    this->addChild(_storePanel, 3);

    // 商店面板高度(屏幕的30%)
    float panelHeight = visibleSize.height * 0.3f;

    // 商店面板背景
    auto panelBg = LayerColor::create(Color4B(50, 50, 50, 200), visibleSize.width, panelHeight);
    panelBg->setPosition(Vec2(0, 0));
    _storePanel->addChild(panelBg);

    // 创建上70%区域的遮罩(修复关闭逻辑)
    auto mask = LayerColor::create(Color4B(0, 0, 0, 100));
    mask->setContentSize(Size(visibleSize.width, visibleSize.height - panelHeight));
    mask->setPosition(Vec2(0, panelHeight)); // 定位在商店面板上方

    auto maskListener = EventListenerTouchOneByOne::create();
    maskListener->setSwallowTouches(true);
    maskListener->onTouchBegan = [this, mask](Touch* touch, Event* event) {
        // 检查点击是否在遮罩范围内
        Vec2 touchLocation = touch->getLocation();
        Vec2 maskLocation = mask->convertToNodeSpace(touchLocation);
        Rect maskRect = Rect(0, 0, mask->getContentSize().width, mask->getContentSize().height);

        if (maskRect.containsPoint(maskLocation)) {
            toggleStorePanel();
            return true;
        }
        return false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(maskListener, mask);
    _storePanel->addChild(mask);

    // 创建建筑选择滚动视图
    _buildingScrollView = ScrollView::create();
    _buildingScrollView->setContentSize(Size(visibleSize.width, panelHeight));
    _buildingScrollView->setPosition(Vec2(0, 0));
    _buildingScrollView->setDirection(ScrollView::Direction::HORIZONTAL);
    _storePanel->addChild(_buildingScrollView);

    // 添加建筑图标到滚动视图
    initBuildingScrollContent();

    // 给滚动视图添加触摸事件防止事件穿透
    auto scrollListener = EventListenerTouchOneByOne::create();
    scrollListener->setSwallowTouches(true);
    scrollListener->onTouchBegan = [](Touch* touch, Event* event) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(scrollListener, _buildingScrollView);

    return true;
}

void Base::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}

void Base::menuBackCallback(Ref* pSender)
{
    auto helloWorldScene = HelloWorld::createScene();
    if (helloWorldScene)
    {
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, helloWorldScene));
    }
}

bool Base::onMouseScroll(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (!background) return false;

    float scrollY = e->getScrollY();

    if (scrollY > 0)
    {
        scaleFactor *= 0.9f;
        if (scaleFactor < 1.0f)
            scaleFactor = 1.0f;
    }
    else if (scrollY < 0)
    {
        scaleFactor *= 1.1f;
        if (scaleFactor > 3.0f)
            scaleFactor = 3.0f;
    }

    background->setScale(scaleFactor);
    constrainBackgroundPosition();

    return true;
}

bool Base::onMouseMove(Event* event)
{
    if (!isDragging || !background) return false;

    EventMouse* e = static_cast<EventMouse*>(event);
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 delta = currentMousePos - lastMousePos;

    Vec2 newPos = backgroundPos + delta;
    backgroundPos = newPos;
    constrainBackgroundPosition();
    background->setPosition(backgroundPos);

    lastMousePos = currentMousePos;
    return true;
}

void Base::constrainBackgroundPosition()
{
    if (!background) return;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    const float originalBgWidth = 1824.0f;
    const float originalBgHeight = 1398.0f;

    float scaledBgWidth = originalBgWidth * scaleFactor;
    float scaledBgHeight = originalBgHeight * scaleFactor;

    float gameMinX = origin.x;
    float gameMaxX = origin.x + visibleSize.width;
    float gameMinY = origin.y;
    float gameMaxY = origin.y + visibleSize.height;

    float minX = gameMinX + scaledBgWidth / 2;
    float maxX = gameMaxX - scaledBgWidth / 2;
    float minY = gameMinY + scaledBgHeight / 2;
    float maxY = gameMaxY - scaledBgHeight / 2;

    backgroundPos.x = clampf(backgroundPos.x, minX, maxX);
    backgroundPos.y = clampf(backgroundPos.y, minY, maxY);
    background->setPosition(backgroundPos);
}

bool Base::onMouseDown(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        // 获取鼠标位置，转换为UI坐标系
        Vec2 mousePos = this->convertToNodeSpace(Vec2(e->getCursorX(), e->getCursorY()));

        // 检查是否点击商店按钮
        if (_storeButton && _storeButton->getBoundingBox().containsPoint(mousePos)) {
            onStoreButtonClicked(_storeButton);
            return true;
        }

        // 检查是否点击关闭按钮
        auto closeItem = this->getChildByTag(999);
        if (closeItem && closeItem->getBoundingBox().containsPoint(mousePos)) {
            menuCloseCallback(closeItem);
            return true;
        }

        isDragging = true;
        lastMousePos = mousePos;
    }
    return true;
}

bool Base::onMouseUp(Event* event)
{
    EventMouse* e = static_cast<EventMouse*>(event);
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
    {
        isDragging = false;
    }
    return true;
}

// 商店初始化实现
void Base::initBuildingScrollContent() {
    const float spacing = 20.0f;
    const float buildingIconSize = 100.0f;

    std::vector<std::pair<BuildingType, std::string>> buildings = {
        {BuildingType::GOLD_MINE, "GoldMine.png"},
        {BuildingType::ELIXIR_COLLECTOR, "ElixirCollector.png"},
        // 可以添加更多建筑
    };

    _scrollContentWidth = buildings.size() * (buildingIconSize + spacing) - spacing;

    auto contentLayer = Layer::create();
    contentLayer->setContentSize(Size(_scrollContentWidth, _buildingScrollView->getContentSize().height));
    _buildingScrollView->addChild(contentLayer);
    _buildingScrollView->setInnerContainerSize(contentLayer->getContentSize());

    for (size_t i = 0; i < buildings.size(); ++i) {
        auto buildingBtn = Button::create(buildings[i].second);
        buildingBtn->setScale(buildingIconSize / buildingBtn->getContentSize().width);
        buildingBtn->setPosition(Vec2(
            i * (buildingIconSize + spacing) + buildingIconSize / 2,
            _buildingScrollView->getContentSize().height / 2
        ));

        buildingBtn->setTag(static_cast<int>(buildings[i].first));
        buildingBtn->addClickEventListener(CC_CALLBACK_1(Base::onBuildingSelected, this));
        contentLayer->addChild(buildingBtn);
    }
}

void Base::onStoreButtonClicked(Ref* sender) {
    toggleStorePanel();
}

void Base::toggleStorePanel() {
    _isStoreOpen = !_isStoreOpen;

    if (_isStoreOpen) {
        _storePanel->setVisible(true);
        _storePanel->setPositionY(-_buildingScrollView->getContentSize().height);
        _storePanel->runAction(MoveTo::create(0.3f, Vec2::ZERO));
    }
    else {
        _storePanel->runAction(Sequence::create(
            MoveTo::create(0.3f, Vec2(0, -_buildingScrollView->getContentSize().height)),
            CallFunc::create([this]() {
                _storePanel->setVisible(false);
                }),
            nullptr
        ));
    }
}

bool Base::onScrollTouchBegan(Touch* touch, Event* event) {
    Vec2 touchLocation = touch->getLocation();
    Vec2 scrollLocation = _buildingScrollView->convertToNodeSpace(touchLocation);
    Rect scrollRect = Rect(0, 0, _buildingScrollView->getContentSize().width, _buildingScrollView->getContentSize().height);

    if (scrollRect.containsPoint(scrollLocation)) {
        _scrollStartPos = touchLocation;
        return true;
    }
    return false;
}

bool Base::onScrollTouchMoved(Touch* touch, Event* event) {
    Vec2 currentPos = touch->getLocation();
    Vec2 delta = currentPos - _scrollStartPos;

    Vec2 innerPos = _buildingScrollView->getInnerContainerPosition();
    _buildingScrollView->setInnerContainerPosition(Vec2(innerPos.x + delta.x, innerPos.y));

    _scrollStartPos = currentPos;
    return true;
}

bool Base::onScrollTouchEnded(Touch* touch, Event* event) {
    return true;
}

void Base::onBuildingSelected(Ref* sender) {
    Button* buildingBtn = dynamic_cast<Button*>(sender);
    if (!buildingBtn) return;

    BuildingType type = static_cast<BuildingType>(buildingBtn->getTag());
    bool canBuild = false;
    std::string errorMsg;

    // 检查资源是否足够
    switch (type) {
        case BuildingType::GOLD_MINE:
            if (_gold >= GOLD_MINE_CONSUME) {
                canBuild = true;
                _gold -= GOLD_MINE_CONSUME;
            }
            else {
                errorMsg = "建造失败，黄金资源不足";
            }
            break;
        case BuildingType::ELIXIR_COLLECTOR:
            if (_elixir >= ELIXIR_COLLECTOR_CONSUME) {
                canBuild = true;
                _elixir -= ELIXIR_COLLECTOR_CONSUME;
            }
            else {
                errorMsg = "建造失败，圣水资源不足";
            }
            break;
        default:
            errorMsg = "未知错误";
            break;
    }

    // 资源不足时显示提示（修复显示问题）
    if (!canBuild) {
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();

        auto label = Label::createWithTTF(errorMsg, "fonts/Marker Felt.ttf", 36);
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
        label->setColor(Color3B::RED);
        this->addChild(label, 100); // 大幅提高层级确保可见

        // 2秒后自动移除提示
        label->runAction(Sequence::create(
            DelayTime::create(2.0f),
            FadeOut::create(0.5f),
            RemoveSelf::create(),
            nullptr
        ));
        return;
    }

    // 资源足够时创建建筑
    Vec2 visibleCenter = Director::getInstance()->getVisibleSize() / 2;
    Architecture* newBuilding = Architecture::create(type, 1);

    if (newBuilding) {
        float randomX = visibleCenter.x + (rand() % 200 - 100);
        float randomY = visibleCenter.y + (rand() % 200 - 100);

        newBuilding->setPosition(Vec2(randomX, randomY));

        if (type == BuildingType::GOLD_MINE) {
            newBuilding->setResourceCallback([this](ResourceType resType, int amount) {
                _gold += amount;
                CCLOG("Gold: %d", _gold);
                });
        }
        else if (type == BuildingType::ELIXIR_COLLECTOR) {
            newBuilding->setResourceCallback([this](ResourceType resType, int amount) {
                _elixir += amount;
                CCLOG("Elixir: %d", _elixir);
                });
        }

        this->addChild(newBuilding, 1);
    }

    toggleStorePanel();
}