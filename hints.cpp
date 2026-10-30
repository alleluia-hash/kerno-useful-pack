#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

// Hand-written hint per level. Type a hint on the level page; it is saved
// and shown after each death on that level.
class $modify(KernoHintLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        if (!level) return true;

        int levelId = level->m_levelID.value();
        std::string saved = Mod::get()->getSavedValue<std::string>("hint-" + std::to_string(levelId), "");

        auto input = TextInput::create(220.f, "Your hint for this level", "bigFont.fnt");
        input->setMaxCharCount(120);
        input->setString(saved, false);
        input->setCallback([levelId](std::string const& text) {
            Mod::get()->setSavedValue<std::string>("hint-" + std::to_string(levelId), text);
        });

        // Position is a guess; adjust it if it overlaps other buttons.
        auto size = CCDirector::sharedDirector()->getWinSize();
        input->setPosition({size.width * 0.5f, size.height * 0.2f});
        this->addChild(input);
        return true;
    }
};
