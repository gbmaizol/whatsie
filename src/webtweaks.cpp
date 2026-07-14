#include "webtweaks.h"
#include "settingsmanager.h"

#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

static const char kScriptName[] = "whatsie-web-tweaks";

// __FLAGS__ becomes a JSON object of the enabled tweaks. Behavior is gated by
// the LIVE flags object (window.__whatsieWebTweaks) rather than a captured
// boolean, so re-running this script on a loaded page toggles the tweak
// without a reload. Every DOM access is wrapped so a stale selector can never
// break the page.
static const char kScriptTemplate[] = R"JS(
(function () {
  'use strict';
  var FLAGS = __FLAGS__;
  var W = window.__whatsieWebTweaks;
  if (W) {
    W.dismissExpressionsPanel = FLAGS.dismissExpressionsPanel;  // live update
    W.themeToggleButton = FLAGS.themeToggleButton;
  } else {
    W = window.__whatsieWebTweaks = FLAGS;
  }
  if (window.__whatsieWebTweaksReady) {
    // Re-run from Settings: the listeners are already in place, but the button
    // must be added or removed to match the flag that just changed.
    if (window.__whatsieInstallThemeButton) window.__whatsieInstallThemeButton();
    return;
  }
  window.__whatsieWebTweaksReady = true;

  // ── Dismiss the expressions (emoji/GIF/sticker) panel on outside click. ──
  // WhatsApp keeps it open until its button is pressed again. The panel mount
  // node '#expressions-panel-container' always exists (empty, height 0); it is
  // OPEN exactly when its child has a subtree. Close via a synthetic Escape.
  var isOpen = function (c) {
    return !!(c && c.firstElementChild && c.firstElementChild.childElementCount > 0);
  };
  document.addEventListener('pointerdown', function (ev) {
    try {
      if (!W.dismissExpressionsPanel) return;
      var panel = document.querySelector('#expressions-panel-container');
      if (!isOpen(panel) || panel.contains(ev.target)) return;
      var btn = ev.target.closest && ev.target.closest('button');
      if (btn && /emoji|gif|sticker/i.test(btn.getAttribute('aria-label') || ''))
        return;
      document.dispatchEvent(new KeyboardEvent('keydown', {
        key: 'Escape', code: 'Escape', keyCode: 27, which: 27, bubbles: true,
      }));
    } catch (e) { /* never break the page */ }
  }, true);

  // ── A light/dark toggle in WhatsApp's own nav rail, above the avatar. ──────
  // The rail's classes are obfuscated and change with every WhatsApp build, so
  // nothing here is matched by class: the avatar is found as the last button in
  // the rail that holds an <img>, and our button is a CLONE of a sibling, which
  // is what makes it look native without hardcoding a single style.
  var BTN_ID = 'whatsie-theme-toggle';
  var ICON = {
    // Sun: shown while dark, i.e. click to go light.
    light: '<circle cx="12" cy="12" r="4.2"/><g stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><path d="M12 2.6v2.2M12 19.2v2.2M2.6 12h2.2M19.2 12h2.2M5.3 5.3l1.6 1.6M17.1 17.1l1.6 1.6M18.7 5.3l-1.6 1.6M6.9 17.1l-1.6 1.6"/></g>',
    // Moon: shown while light, i.e. click to go dark.
    dark: '<path d="M20.3 14.6A8.6 8.6 0 0 1 9.4 3.7a8.6 8.6 0 1 0 10.9 10.9z"/>',
  };
  var isDark = function () {
    try {
      return document.documentElement.classList.contains('dark') ||
             document.body.classList.contains('dark') ||
             (localStorage.getItem('theme') || '').indexOf('dark') !== -1;
    } catch (e) { return false; }
  };
  var paint = function (button) {
    var dark = isDark();
    var svg = button.querySelector('svg');
    if (!svg) return;
    svg.setAttribute('viewBox', '0 0 24 24');
    svg.setAttribute('fill', 'currentColor');
    svg.innerHTML = dark ? ICON.light : ICON.dark;
    var label = dark ? 'Switch to light theme' : 'Switch to dark theme';
    button.setAttribute('aria-label', label);
    button.setAttribute('title', label);
  };
  // The buttons of the narrow left column, in document order.
  var railButtons = function () {
    return Array.prototype.slice.call(document.querySelectorAll('button'))
      .filter(function (b) {
        var r = b.getBoundingClientRect();
        return r.width > 0 && r.width <= 72 && r.left < 80;
      });
  };
  // Each rail entry is wrapped several levels deep (button > div > span > div),
  // so the avatar's own parent holds nothing but the avatar. Climb until the
  // level where entries are actually siblings of each other.
  var wrapperSharedWith = function (node, other) {
    while (node.parentElement) {
      if (node.parentElement.contains(other)) return node;
      node = node.parentElement;
    }
    return null;
  };
  var install = function () {
    try {
      if (!W.themeToggleButton) {
        var old = document.getElementById(BTN_ID);
        if (old) old.remove();
        return;
      }
      var existing = document.getElementById(BTN_ID);
      if (existing && existing.isConnected) {
        paint(existing.querySelector('button') || existing);
        return;
      }

      var rail = railButtons();
      var avatar = null, template = null;
      for (var i = rail.length - 1; i >= 0; i--) {
        if (!avatar && rail[i].querySelector('img')) { avatar = rail[i]; continue; }
        if (avatar && !template && rail[i].querySelector('svg') &&
            !rail[i].querySelector('img')) { template = rail[i]; }
      }
      if (!avatar || !template) return;

      var avatarWrapper = wrapperSharedWith(avatar, template);
      var templateWrapper = wrapperSharedWith(template, avatar);
      if (!avatarWrapper || !templateWrapper ||
          avatarWrapper.parentElement !== templateWrapper.parentElement) return;

      // Clone a whole neighbouring entry rather than styling one from scratch:
      // every class in this rail is obfuscated and changes with each WhatsApp
      // build, and a clone is immune to that. cloneNode drops event listeners,
      // which is exactly what is wanted here.
      var entry = templateWrapper.cloneNode(true);
      entry.id = BTN_ID;
      var button = entry.querySelector('button') || entry;
      button.removeAttribute('data-navbar-item');
      // The entry it was copied from may have been the selected one.
      button.removeAttribute('aria-pressed');
      button.removeAttribute('aria-selected');
      button.removeAttribute('aria-current');
      paint(button);
      entry.addEventListener('click', function (ev) {
        ev.preventDefault();
        ev.stopPropagation();
        if (window.__whatsieBridge && window.__whatsieBridge.toggleTheme)
          window.__whatsieBridge.toggleTheme();
      }, true);

      avatarWrapper.parentElement.insertBefore(entry, avatarWrapper);
    } catch (e) { /* never break the page */ }
  };

  window.__whatsieInstallThemeButton = install;

  // WhatsApp rebuilds the rail on navigation, so keep putting the button back.
  install();
  var scheduled = false;
  new MutationObserver(function () {
    if (scheduled) return;
    scheduled = true;
    setTimeout(function () { scheduled = false; install(); }, 300);
  }).observe(document.body, {childList: true, subtree: true});

  // The icon has to follow the theme even when nothing else on the page moves,
  // so watch the class that carries it rather than waiting for a DOM change.
  new MutationObserver(function () {
    var entry = document.getElementById(BTN_ID);
    if (entry) paint(entry.querySelector('button') || entry);
  }).observe(document.documentElement, {
    attributes: true, attributeFilter: ['class'],
  });
})();
)JS";

namespace WebTweaks {

static const char *jsBool(bool value) { return value ? "true" : "false"; }

QString scriptSource() {
  QSettings &s = SettingsManager::instance().settings();
  const bool dismiss =
      s.value(QStringLiteral("webtweaks/dismissExpressionsPanel"), false).toBool();
  const bool themeButton =
      s.value(QStringLiteral("webtweaks/themeToggleButton"), true).toBool();

  const QString flags =
      QStringLiteral("{\"dismissExpressionsPanel\":%1,\"themeToggleButton\":%2}")
          .arg(QLatin1String(jsBool(dismiss)), QLatin1String(jsBool(themeButton)));

  QString source = QString::fromLatin1(kScriptTemplate);
  source.replace(QLatin1String("__FLAGS__"), flags);
  return source;
}

void install(QWebEngineProfile *profile) {
  auto *scripts = profile->scripts();
  const auto existing = scripts->find(QLatin1String(kScriptName));
  for (const auto &script : existing)
    scripts->remove(script);

  QSettings &s = SettingsManager::instance().settings();
  const bool dismiss =
      s.value(QStringLiteral("webtweaks/dismissExpressionsPanel"), false).toBool();
  const bool themeButton =
      s.value(QStringLiteral("webtweaks/themeToggleButton"), true).toBool();
  if (!dismiss && !themeButton)
    return; // nothing enabled → do not inject on fresh loads

  QWebEngineScript script;
  script.setName(QLatin1String(kScriptName));
  script.setSourceCode(scriptSource());
  script.setInjectionPoint(QWebEngineScript::DocumentReady);
  script.setWorldId(QWebEngineScript::MainWorld);
  script.setRunsOnSubFrames(false);
  scripts->insert(script);
}

} // namespace WebTweaks
