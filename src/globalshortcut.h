#ifndef GLOBALSHORTCUT_H
#define GLOBALSHORTCUT_H

#include <QAbstractNativeEventFilter>
#include <QObject>

#if defined(Q_OS_LINUX)
#include <QDBusObjectPath>
#include <QVariantMap>
#endif

// A system-wide hotkey to raise the window (Ctrl+Alt+W).
//
// It prefers the XDG GlobalShortcuts portal, which is the only thing that works
// on Wayland and also works on X11 wherever a portal backend implements it
// (KDE Plasma, GNOME). Where the portal is missing it falls back to a raw X11
// key grab (X11 sessions only). If neither is available — e.g. Wayland without
// the portal — tryRegister() returns false and a `whatly -w` desktop shortcut
// is the alternative. activated() fires whenever the combo is pressed.
class GlobalShortcut : public QObject, public QAbstractNativeEventFilter {
  Q_OBJECT
public:
  explicit GlobalShortcut(QObject *parent = nullptr);
  ~GlobalShortcut() override;

  // Returns true if a backend accepted the shortcut. The portal path is
  // asynchronous, so a true there only means the request was sent — the bind
  // (and, on KDE, a one-time confirmation dialog) completes shortly after.
  bool tryRegister();

  bool nativeEventFilter(const QByteArray &eventType, void *message,
                         qintptr *result) override;

signals:
  void activated();

#if defined(Q_OS_LINUX)
private slots:
  void onCreateSessionResponse(uint response, const QVariantMap &results);
  void onBindResponse(uint response, const QVariantMap &results);
  void onPortalActivated(const QDBusObjectPath &sessionHandle,
                         const QString &shortcutId, qulonglong timestamp,
                         const QVariantMap &options);
#endif

private:
  bool tryPortal();
  bool tryX11();
  void ungrabX11();

#if defined(Q_OS_LINUX)
  QString m_sessionHandle;  // portal session object path, once created
#endif
  quint32 m_keycode = 0;    // X11 fallback: keycode of 'W'
  quint32 m_modifiers = 0;  // X11 fallback: Control + Alt
  bool m_x11Registered = false;
};

#endif // GLOBALSHORTCUT_H
