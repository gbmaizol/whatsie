#ifndef PAGEBRIDGE_H
#define PAGEBRIDGE_H

#include <QObject>

// The one object WhatsApp's page can reach, over QWebChannel. Buttons WhatSie
// injects into the page (see WebTweaks) call into here; nothing else is
// exposed, and the page cannot reach anything this class does not declare as a
// slot.
class PageBridge : public QObject {
  Q_OBJECT
public:
  explicit PageBridge(QObject *parent = nullptr) : QObject(parent) {}

public slots:
  // Invoked by the theme button injected next to the profile avatar.
  void toggleTheme() { emit themeToggleRequested(); }

signals:
  void themeToggleRequested();
};

#endif // PAGEBRIDGE_H
