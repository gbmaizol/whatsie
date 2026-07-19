#ifndef PORTALNOTIFICATION_H
#define PORTALNOTIFICATION_H

#include <QObject>
#include <QString>

// Desktop notifications through the XDG desktop portal
// (org.freedesktop.portal.Notification). This is the sandbox-friendly path: a
// Flatpak build cannot always reach org.freedesktop.Notifications directly, but
// it can always reach the portal. Outside a sandbox the portal works too, so it
// is a valid backend everywhere the portal is running.
//
// Linux only. On other platforms the methods compile to safe no-ops.
class PortalNotification : public QObject {
  Q_OBJECT
public:
  explicit PortalNotification(QObject *parent = nullptr);

  // True when the app is running inside a Flatpak sandbox.
  static bool inFlatpak();

  // True when the portal Notification interface is reachable on the session bus.
  static bool isAvailable();

  // Post a notification. `id` is a caller-chosen stable key (posting again with
  // the same id replaces the previous one). Returns true if it was dispatched.
  bool send(const QString &id, const QString &title, const QString &body);

  // Withdraw a previously posted notification.
  void remove(const QString &id);

signals:
  // Emitted when the user activates the notification (its default action or the
  // "open" button).
  void activated(const QString &id);

private slots:
  void onActionInvoked(const QString &id, const QString &action);

private:
  bool m_connected = false;
};

#endif // PORTALNOTIFICATION_H
