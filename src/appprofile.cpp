#include "appprofile.h"

#include <QRegularExpression>

namespace {

QString g_id;
bool g_initialised = false;

// Anything the user types becomes part of a settings filename, a WebEngine
// directory name and a D-Bus-ish instance key, so keep it to a safe slug rather
// than trusting the input.
QString sanitise(QString value) {
  value = value.trimmed().toLower();
  value.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]")),
                QStringLiteral("-"));
  value = value.left(32);
  if (value == QLatin1String("default"))
    return QString();
  return value;
}

} // namespace

namespace AppProfile {

void initFromArgs(int argc, char *argv[]) {
  g_initialised = true;
  for (int i = 1; i < argc; ++i) {
    const QString arg = QString::fromLocal8Bit(argv[i]);
    if (arg.startsWith(QStringLiteral("--profile="))) {
      g_id = sanitise(arg.mid(10));
      return;
    }
    if ((arg == QStringLiteral("--profile") || arg == QStringLiteral("-p")) &&
        i + 1 < argc) {
      g_id = sanitise(QString::fromLocal8Bit(argv[i + 1]));
      return;
    }
  }
}

QString id() { return g_id; }

bool isDefault() { return g_id.isEmpty(); }

QString suffix() {
  return g_id.isEmpty() ? QString() : QLatin1Char('-') + g_id;
}

QString label() {
  return g_id.isEmpty() ? QString()
                        : QStringLiteral(" (") + g_id + QLatin1Char(')');
}

} // namespace AppProfile
