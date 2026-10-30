# Kerno's Useful Pack (Geode mod), v1.5

Features:
- **Last try summary**: progress on the last attempt, compared with the one before.
- **Pace**: progress per minute for each attempt, and your best pace per level.
- **Best %** and **total playtime** per level, counted only while playing.
- **Death hot spot** and **killer log**, saved per level.
- **Milestone popups** at 25%, 50%, 75%, and 100%.
- **Death feedback**: how close you were to your best when you died, plus a tip (generic, based on what killed you).
- **Personal difficulty tier**: a button on the level page. Click to cycle Easy ... Extreme Demon. Saved per level.

- **Hand-written hints**: a text box on the level page. Your hint is saved per level and shown after each death. It is on the level page, not in a 💡 menu.

- **Refresh button** on the main menu: re-requests the Recent online levels with `GameLevelManager::get()->getOnlineLevels(GJSearchObject::create(SearchType::Recent))`. The button position is a guess.
- **PNG import (partial)**: on your own profile, an "Import PNG" button picks a PNG and saves it per account. It shows as an overlay on the profile page at a guessed position. It does not replace GD's own 👤 icon yet.

Not done yet:
- Replacing the 👤 icon itself (needs the ProfilePage icon field).
- PNG import for the account icon (needs the file picker and the profile page internals).
- The tier button is a click-to-cycle button, not a popup list, and it sits at a guessed position on the level page.

Build:
1. Install the Geode SDK and set `GEODE_SDK` to its folder, or use the `geode` CLI: `geode build`.
2. Copy the built `.geode` file into your Geometry Dash `geode/mods` folder.

Status: not compiled or tested. Check these if the build fails:
- `m_level->m_levelID.value()`, `GameObject::m_objectID`
- `LevelInfoLayer::init(GJGameLevel*, bool)`, `CCMenuItemLabel::create`, `CCLabelBMFont`, `menu_selector`
- `ProfilePage::init`, `geode::utils::file::pick`, `geode::async::spawn`, `FilePickOptions` (from your snippet)
- `MenuLayer::init`, `GameLevelManager::get`, `getOnlineLevels`, `GJSearchObject::create`, `SearchType::Recent`
- `TextInput::create`, `setCallback`, `setString`, `setMaxCharCount` (from the Geode docs)
- `Mod::get()->getSavedValue` / `setSavedValue`, `Notification::create(...)`
- `destroyPlayer`, `postUpdate`, `resetLevel`, `getCurrentPercentInt`, `isGameplayActive`
