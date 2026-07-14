#pragma once

#include <QWebEngineProfile>
#include <QWebEngineSettings>

// Singleton that owns the single persistent QWebEngineProfile for the app.
// Call instance() before creating any QWebEnginePage.
class WebEngineProfileManager {
public:
    static WebEngineProfileManager &instance();

    QWebEngineProfile *profile() const;

    // Re-reads the user-configurable settings — user agent, autoplay, spell
    // check — and the injected scripts from QSettings, and applies them to the
    // profile. Call whenever any of them changes, instead of recreating the
    // page.
    void applyUserSettings();

private:
    WebEngineProfileManager();
    ~WebEngineProfileManager();

    QWebEngineProfile *m_profile;
};
