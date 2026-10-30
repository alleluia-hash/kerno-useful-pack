#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

namespace kerno {
    static const char* TIERS[] = {
        "Not set", "Easy", "Normal", "Hard", "Harder", "Insane",
        "Easy Demon", "Medium Demon", "Hard Demon", "Insane Demon", "Extreme Demon"
    };
    constexpr int TIER_COUNT = 11;
}

// Personal difficulty tier, shown on the level page. Click to cycle through tiers.
class $modify(KernoTierLayer, LevelInfoLayer) {
    struct Fields {
        int levelId = 0;
        int tier = 0;
        CCLabelBMFont* tierLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        if (!level) return true;

        m_fields->levelId = level->m_levelID.value();
        m_fields->tier = Mod::get()->getSavedValue<int>("tier-" + std::to_string(m_fields->levelId), 0);

        auto label = CCLabelBMFont::create("", "bigFont.fnt");
        label->setScale(0.5f);
        m_fields->tierLabel = label;

        auto item = CCMenuItemLabel::create(label, this, menu_selector(KernoTierLayer::onCycleTier));
        auto menu = CCMenu::create();
        menu->addChild(item);

        // Position is a guess; adjust it if it overlaps other buttons.
        auto size = CCDirector::sharedDirector()->getWinSize();
        menu->setPosition(size.width * 0.5f, size.height * 0.12f);
        this->addChild(menu);

        this->refreshTierText();
        return true;
    }

    void refreshTierText() {
        std::string text = std::string("Your tier: ") + kerno::TIERS[m_fields->tier];
        m_fields->tierLabel->setString(text.c_str());
    }

    void onCycleTier(CCObject*) {
        m_fields->tier = (m_fields->tier + 1) % kerno::TIER_COUNT;
        Mod::get()->setSavedValue<int>("tier-" + std::to_string(m_fields->levelId), m_fields->tier);
        this->refreshTierText();
    }
};
