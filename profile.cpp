#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/utils/file.hpp>
#include <Geode/utils/async.hpp>
#include <filesystem>

using namespace geode::prelude;

// On your own profile, adds an "Import PNG" button. It picks a PNG, saves its path
// for this account, and shows the image on the profile page.
// NOTE: this shows the image as an overlay. Replacing GD's own 👤 icon needs the
// ProfilePage icon field, which is not confirmed yet.
class $modify(KernoProfileHook, ProfilePage) {
    bool init(int accountID, bool isOwnProfile) {
        if (!ProfilePage::init(accountID, isOwnProfile)) return false;

        std::string savedPath = Mod::get()->getSavedValue<std::string>("icon-" + std::to_string(accountID), "");
        if (!savedPath.empty()) {
            auto image = CCSprite::create(savedPath.c_str());
            if (image) {
                // Position and scale are guesses.
                auto size = CCDirector::sharedDirector()->getWinSize();
                image->setScale(0.5f);
                image->setPosition({size.width * 0.5f, size.height * 0.8f});
                this->addChild(image);
            }
        }

        if (isOwnProfile) {
            auto sprite = ButtonSprite::create("Import PNG", 120, false, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.7f);
            auto item = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(KernoProfileHook::onImportPng));
            item->setTag(accountID);

            auto menu = CCMenu::create();
            menu->addChild(item);

            // Position is a guess.
            auto size = CCDirector::sharedDirector()->getWinSize();
            menu->setPosition(size.width * 0.5f, size.height * 0.05f);
            this->addChild(menu);
        }
        return true;
    }

    void onImportPng(CCObject* sender) {
        int accountID = static_cast<CCNode*>(sender)->getTag();

        geode::utils::file::FilePickOptions options;
        options.defaultPath = std::filesystem::current_path();
        options.filters = { { "PNG images", {"*.png"} } };

        geode::async::spawn(geode::utils::file::pick(geode::utils::file::PickMode::OpenFile, options))
            .then([accountID](auto result) {
                if (result && result.has_value()) {
                    std::string path = result.value().string();
                    Mod::get()->setSavedValue<std::string>("icon-" + std::to_string(accountID), path);
                    Notification::create("Image saved. Reopen your profile to see it.", NotificationIcon::Success, 2.5f)->show();
                }
            })
            .expect([](std::string const& error) {
                Notification::create("File picker failed: " + error, NotificationIcon::Error, 2.5f)->show();
            });
    }
};
