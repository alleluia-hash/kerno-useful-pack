#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <map>
#include <sstream>
#include <string>

using namespace geode::prelude;

namespace kerno {
    constexpr float BUCKET_WIDTH = 150.f;

    static std::string key(const char* prefix, int levelId) {
        return std::string(prefix) + std::to_string(levelId);
    }

    static std::map<int, int> loadMap(const char* prefix, int levelId) {
        std::map<int, int> out;
        std::string raw = Mod::get()->getSavedValue<std::string>(key(prefix, levelId), "");
        std::stringstream ss(raw);
        std::string item;
        while (std::getline(ss, item, ',')) {
            auto colon = item.find(':');
            if (colon == std::string::npos) continue;
            try {
                out[std::stoi(item.substr(0, colon))] = std::stoi(item.substr(colon + 1));
            } catch (...) {}
        }
        return out;
    }

    static void saveMap(const char* prefix, int levelId, const std::map<int, int>& map) {
        std::string raw;
        for (auto& [k, count] : map) {
            raw += std::to_string(k) + ":" + std::to_string(count) + ",";
        }
        Mod::get()->setSavedValue<std::string>(key(prefix, levelId), raw);
    }

    static int topKey(const std::map<int, int>& map, int& countOut) {
        int best = 0;
        countOut = 0;
        for (auto& [k, count] : map) {
            if (count > countOut) { countOut = count; best = k; }
        }
        return best;
    }
}

class $modify(KernoPlayLayer, PlayLayer) {
    struct Fields {
        float pendingTime = 0.f;
        int lastMilestone = 0;
        int lastDelta = 0;
        float lastPace = 0.f;
    };

    void resetLevel() {
        int levelId = m_level ? m_level->m_levelID.value() : 0;

        if (m_level) {
            int percent = this->getCurrentPercentInt();

            int previous = Mod::get()->getSavedValue<int>(kerno::key("lastattempt-", levelId), 0);
            m_fields->lastDelta = percent - previous;
            Mod::get()->setSavedValue<int>(kerno::key("lastattempt-", levelId), percent);

            int best = Mod::get()->getSavedValue<int>(kerno::key("best-", levelId), 0);
            if (percent > best) {
                Mod::get()->setSavedValue<int>(kerno::key("best-", levelId), percent);
            }

            float minutes = m_fields->pendingTime / 60.f;
            m_fields->lastPace = minutes > 0.05f ? percent / minutes : 0.f;
            float bestPace = Mod::get()->getSavedValue<float>(kerno::key("bestpace-", levelId), 0.f);
            if (m_fields->lastPace > bestPace) {
                Mod::get()->setSavedValue<float>(kerno::key("bestpace-", levelId), m_fields->lastPace);
            }

            float total = Mod::get()->getSavedValue<float>(kerno::key("playtime-", levelId), 0.f);
            Mod::get()->setSavedValue<float>(kerno::key("playtime-", levelId), total + m_fields->pendingTime);
        }
        m_fields->pendingTime = 0.f;
        m_fields->lastMilestone = 0;

        PlayLayer::resetLevel();

        if (!m_level) return;

        int bestPace = static_cast<int>(Mod::get()->getSavedValue<float>(kerno::key("bestpace-", levelId), 0.f));
        int best = Mod::get()->getSavedValue<int>(kerno::key("best-", levelId), 0);

        int hotCount = 0;
        int hotX = static_cast<int>(kerno::topKey(kerno::loadMap("deaths-", levelId), hotCount) * kerno::BUCKET_WIDTH);

        int killerCount = 0;
        int killerId = kerno::topKey(kerno::loadMap("killers-", levelId), killerCount);

        std::string text =
            "Last try " + std::to_string(this->getCurrentPercentInt()) + "% (" +
            (m_fields->lastDelta >= 0 ? "+" : "") + std::to_string(m_fields->lastDelta) + " vs before)" +
            " | Pace " + std::to_string(static_cast<int>(m_fields->lastPace)) + "%/min (best " + std::to_string(bestPace) + ")" +
            " | Best " + std::to_string(best) + "%";
        if (hotCount > 0) {
            text += " | Hot spot x~" + std::to_string(hotX) + " (" + std::to_string(hotCount) + " deaths)";
        }
        if (killerCount > 0) {
            text += " | Top killer: object " + std::to_string(killerId) + " (" + std::to_string(killerCount) + "x)";
        }

        Notification::create(text, NotificationIcon::Info, 4.f)->show();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (this->isGameplayActive() && !m_hasCompletedLevel) {
            m_fields->pendingTime += dt;

            int percent = this->getCurrentPercentInt();
            int reached = 0;
            for (int m : {25, 50, 75, 100}) {
                if (percent >= m) reached = m;
            }
            if (reached > m_fields->lastMilestone) {
                m_fields->lastMilestone = reached;
                Notification::create("Milestone: " + std::to_string(reached) + "%!", NotificationIcon::Success, 1.5f)->show();
            }
        }
    }

    // Death feedback: closeness to your best, plus a tip based on what killed you.
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        if (!player) return;

        int levelId = m_level ? m_level->m_levelID.value() : 0;
        int percent = this->getCurrentPercentInt();
        int best = Mod::get()->getSavedValue<int>(kerno::key("best-", levelId), 0);
        int gap = best - percent;

        auto deaths = kerno::loadMap("deaths-", levelId);
        int bucket = static_cast<int>(player->getPositionX() / kerno::BUCKET_WIDTH);
        deaths[bucket]++;
        kerno::saveMap("deaths-", levelId, deaths);

        int killedByThisCount = 0;
        if (object) {
            auto killers = kerno::loadMap("killers-", levelId);
            killers[object->m_objectID]++;
            killedByThisCount = killers[object->m_objectID];
            kerno::saveMap("killers-", levelId, killers);
        }

        // Closeness line.
        std::string closeness = gap <= 0
            ? "New best range: died at " + std::to_string(percent) + "%"
            : "Died at " + std::to_string(percent) + "%, " + std::to_string(gap) + "% from your best (" + std::to_string(best) + "%)";

        // Tip line, from simple rules.
        std::string tip;
        if (killedByThisCount >= 3) {
            tip = "Tip: this object has ended you " + std::to_string(killedByThisCount) + " times. Slow down before this section, or practice it alone.";
        } else if (gap > 0 && gap <= 2) {
            tip = "Tip: you're very close. Start the jump one frame earlier or later.";
        } else {
            tip = "Tip: try a different timing, or practice this section in isolation.";
        }

        std::string hint = Mod::get()->getSavedValue<std::string>("hint-" + std::to_string(levelId), "");
        if (!hint.empty()) {
            tip += " | Your hint: " + hint;
        }

        Notification::create(closeness + " | " + tip, NotificationIcon::Warning, 4.f)->show();
    }
};
