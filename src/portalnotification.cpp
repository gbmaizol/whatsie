#include "portalnotification.h"

#include <QFile>

#if defined(Q_OS_LINUX)
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QVariantMap>

namespace {
const char kService[] = "org.freedesktop.portal.Desktop";
const char kPath[] = "/org/freedesktop/portal/desktop";
const char kIface[] = "org.freedesktop.portal.Notification";
} // namespace
#endif

PortalNotification::PortalNotification(QObject *parent) : QObject(parent) {
#if defined(Q_OS_LINUX)
  // Listen for the portal's ActionInvoked(s id, s action, av parameter) signal.
  // We ignore the trailing parameter array.
  m_connected = QDBusConnection::sessionBus().connect(
      QLatin1String(kService), QLatin1String(kPath), QLatin1String(kIface),
      QStringLiteral("ActionInvoked"), this,
      SLOT(onActionInvoked(QString, QString)));
#endif
}

bool PortalNotification::inFlatpak() {
  return QFile::exists(QStringLiteral("/.flatpak-info")) ||
         qEnvironmentVariableIsSet("FLATPAK_ID");
}

bool PortalNotification::isAvailable() {
#if defined(Q_OS_LINUX)
  QDBusConnection bus = QDBusConnection::sessionBus();
  if (!bus.isConnected())
    return false;
  QDBusConnectionInterface *conn = bus.interface();
  return conn && conn->isServiceRegistered(QLatin1String(kService)).value();
#else
  return false;
#endif
}

bool PortalNotification::send(const QString &id, const QString &title,
                              const QString &body) {
#if defined(Q_OS_LINUX)
  if (!isAvailable())
    return false;

  QVariantMap n;
  n.insert(QStringLiteral("title"), title);
  n.insert(QStringLiteral("body"), body);
  // A themed icon keyed on the app id; the portal falls back to the app's
  // .desktop icon when the theme has none.
  n.insert(QStringLiteral("default-action"), QStringLiteral("open"));
  n.insert(QStringLiteral("priority"), QStringLiteral("normal"));

  QDBusMessage msg = QDBusMessage::createMethodCall(
      QLatin1String(kService), QLatin1String(kPath), QLatin1String(kIface),
      QStringLiteral("AddNotification"));
  msg << id << n;
  const QDBusMessage reply =
      QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
  return reply.type() != QDBusMessage::ErrorMessage;
#else
  Q_UNUSED(id);
  Q_UNUSED(title);
  Q_UNUSED(body);
  return false;
#endif
}

void PortalNotification::remove(const QString &id) {
#if defined(Q_OS_LINUX)
  if (!isAvailable())
    return;
  QDBusMessage msg = QDBusMessage::createMethodCall(
      QLatin1String(kService), QLatin1String(kPath), QLatin1String(kIface),
      QStringLiteral("RemoveNotification"));
  msg << id;
  QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
#else
  Q_UNUSED(id);
#endif
}

void PortalNotification::onActionInvoked(const QString &id,
                                         const QString &action) {
  // The default action arrives as "default"; our explicit button is "open".
  if (action == QLatin1String("open") || action == QLatin1String("default"))
    emit activated(id);
}
