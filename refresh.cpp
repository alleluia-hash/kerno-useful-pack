#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/GJSearchObject.hpp>

using namespace geode::prelude;

// Adds a "Refresh" button to the main menu. It re-requests the Recent online levels.
// Saved stats are read straight from storage, so they need no reload.
class $modify(KernoRefreshMenu, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto sprite = ButtonSprite::create("Refresh", 120, false, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.7f);
        auto item = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(KernoRefreshMenu::onRefresh));

        auto menu = CCMenu::create();
        menu->addChild(item);

        // Position is a guess; adjust it if it overlaps the Geode button.
        auto size = CCDirector::sharedDirector()->getWinSize();
        menu->setPosition(size.width * 0.12f, size.height * 0.12f);
        this->addChild(menu);
        return true;
    }

    void onRefresh(CCObject*) {
        // Re-request the Recent online levels.
        auto search = GJSearchObject::create(SearchType::Recent);
        GameLevelManager::get()->getOnlineLevels(search);

        Notification::create("Refreshing online levels...", NotificationIcon::Info, 2.f)->show();
    }
};
