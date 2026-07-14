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
    W.privacyBlurButton = FLAGS.privacyBlurButton;
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

  // ── Our own entries in WhatsApp's nav rail, above the avatar. ─────────────
  // The rail's classes are obfuscated and change with every WhatsApp build, so
  // nothing here is matched by class: the avatar is found as the last button in
  // the rail that holds an <img>, and each of our buttons is a CLONE of a
  // neighbouring entry, which is what makes them look native without hardcoding
  // a single style.
  var ICON = {
    // Sun: shown while dark, i.e. click to go light.
    sun: '<circle cx="12" cy="12" r="4.2"/><g stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><path d="M12 2.6v2.2M12 19.2v2.2M2.6 12h2.2M19.2 12h2.2M5.3 5.3l1.6 1.6M17.1 17.1l1.6 1.6M18.7 5.3l-1.6 1.6M6.9 17.1l-1.6 1.6"/></g>',
    // Moon: shown while light, i.e. click to go dark.
    moon: '<path d="M20.3 14.6A8.6 8.6 0 0 1 9.4 3.7a8.6 8.6 0 1 0 10.9 10.9z"/>',
    // Open eye: shown while blurred, i.e. click to reveal.
    eye: '<path d="M12 5C6.9 5 2.7 9.3 1.5 12c1.2 2.7 5.4 7 10.5 7s9.3-4.3 10.5-7C21.3 9.3 17.1 5 12 5zm0 11.5a4.5 4.5 0 1 1 0-9 4.5 4.5 0 0 1 0 9z"/><circle cx="12" cy="12" r="2.3"/>',
    // Struck-through eye: shown while clear, i.e. click to blur.
    eyeOff: '<path d="M12 5C6.9 5 2.7 9.3 1.5 12c.6 1.3 1.9 3 3.7 4.4l2-2A4.5 4.5 0 0 1 12 7.5c.5 0 1 .1 1.5.2l1.8-1.8A11 11 0 0 0 12 5zm7.3 1.3-1.6 1.6c1.4 1.1 2.5 2.5 3 3.1-1.1 2.4-4.7 6-9.7 6-.9 0-1.7-.1-2.5-.3l-1.8 1.8c1.3.4 2.8.7 4.3.7 5.1 0 9.3-4.3 10.5-7-.5-1.2-1.6-2.8-3.2-4.2z"/><path d="M4.3 20.4 3 19.1 19.1 3l1.3 1.3z"/>',
  };
  var isDark = function () {
    try {
      return document.documentElement.classList.contains('dark') ||
             document.body.classList.contains('dark') ||
             (localStorage.getItem('theme') || '').indexOf('dark') !== -1;
    } catch (e) { return false; }
  };
  // The blur is a stylesheet the app injects, so the page can read its own
  // state off it — no second channel to keep in sync with the app.
  var isBlurred = function () {
    var style = document.getElementById('whatsie-privacy-blur');
    return !!(style && style.textContent);
  };

  // Every button we add: what it looks like right now, and what a click does.
  var BUTTONS = [
    {
      id: 'whatsie-theme-toggle',
      enabled: function () { return W.themeToggleButton; },
      icon: function () { return isDark() ? ICON.sun : ICON.moon; },
      label: function () {
        return isDark() ? 'Switch to light theme' : 'Switch to dark theme';
      },
      click: function () {
        if (window.__whatsieBridge && window.__whatsieBridge.toggleTheme)
          window.__whatsieBridge.toggleTheme();
      },
    },
    {
      id: 'whatsie-blur-toggle',
      enabled: function () { return W.privacyBlurButton; },
      icon: function () { return isBlurred() ? ICON.eye : ICON.eyeOff; },
      label: function () {
        return isBlurred() ? 'Show the chats' : 'Blur the chats';
      },
      click: function () {
        if (window.__whatsieBridge && window.__whatsieBridge.togglePrivacyBlur)
          window.__whatsieBridge.togglePrivacyBlur();
      },
    },
  ];

  var paint = function (spec, button) {
    var svg = button.querySelector('svg');
    if (!svg) return;
    svg.setAttribute('viewBox', '0 0 24 24');
    svg.setAttribute('fill', 'currentColor');
    svg.innerHTML = spec.icon();
    var label = spec.label();
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
      // Each one goes directly above the avatar, so inserting them in order
      // leaves them on screen in the order they are listed above.
      for (var n = 0; n < BUTTONS.length; n++) {
        var spec = BUTTONS[n];
        var existing = document.getElementById(spec.id);

        if (!spec.enabled()) {
          if (existing) existing.remove();
          continue;
        }
        if (existing && existing.isConnected) {
          paint(spec, existing.querySelector('button') || existing);
          continue;
        }

        var rail = railButtons();
        var avatar = null, template = null;
        for (var i = rail.length - 1; i >= 0; i--) {
          if (!avatar && rail[i].querySelector('img')) { avatar = rail[i]; continue; }
          if (avatar && !template && rail[i].querySelector('svg') &&
              !rail[i].querySelector('img') && !rail[i].closest('[id^="whatsie-"]'))
            template = rail[i];
        }
        if (!avatar || !template) continue;

        var avatarWrapper = wrapperSharedWith(avatar, template);
        var templateWrapper = wrapperSharedWith(template, avatar);
        if (!avatarWrapper || !templateWrapper ||
            avatarWrapper.parentElement !== templateWrapper.parentElement) continue;

        // Clone a whole neighbouring entry rather than styling one from scratch:
        // every class in this rail is obfuscated and changes with each WhatsApp
        // build, and a clone is immune to that. cloneNode drops event listeners,
        // which is exactly what is wanted here.
        var entry = templateWrapper.cloneNode(true);
        entry.id = spec.id;
        var button = entry.querySelector('button') || entry;
        button.removeAttribute('data-navbar-item');
        // The entry it was copied from may have been the selected one.
        button.removeAttribute('aria-pressed');
        button.removeAttribute('aria-selected');
        button.removeAttribute('aria-current');
        paint(spec, button);
        entry.addEventListener('click', (function (s) {
          return function (ev) {
            ev.preventDefault();
            ev.stopPropagation();
            s.click();
          };
        })(spec), true);

        avatarWrapper.parentElement.insertBefore(entry, avatarWrapper);
      }
    } catch (e) { /* never break the page */ }
  };

  var repaint = function () {
    for (var n = 0; n < BUTTONS.length; n++) {
      var entry = document.getElementById(BUTTONS[n].id);
      if (entry) paint(BUTTONS[n], entry.querySelector('button') || entry);
    }
  };

  window.__whatsieInstallThemeButton = install;

  // WhatsApp rebuilds the rail on navigation, so keep putting the buttons back.
  install();
  var scheduled = false;
  new MutationObserver(function () {
    if (scheduled) return;
    scheduled = true;
    setTimeout(function () { scheduled = false; install(); }, 300);
  }).observe(document.body, {childList: true, subtree: true});

  // The icons have to follow the state they report even when nothing else on
  // the page moves: the theme lives in a class on <html>, and the blur in a
  // stylesheet in <head>. Watch both rather than waiting for a DOM change
  // elsewhere.
  new MutationObserver(repaint).observe(document.documentElement, {
    attributes: true, attributeFilter: ['class'],
  });
  new MutationObserver(repaint).observe(document.head, {
    childList: true, subtree: true, characterData: true,
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
  const bool blurButton =
      s.value(QStringLiteral("webtweaks/privacyBlurButton"), true).toBool();

  const QString flags =
      QStringLiteral("{\"dismissExpressionsPanel\":%1,\"themeToggleButton\":%2,"
                     "\"privacyBlurButton\":%3}")
          .arg(QLatin1String(jsBool(dismiss)), QLatin1String(jsBool(themeButton)),
               QLatin1String(jsBool(blurButton)));

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
  const bool blurButton =
      s.value(QStringLiteral("webtweaks/privacyBlurButton"), true).toBool();
  if (!dismiss && !themeButton && !blurButton)
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
