#ifndef PRIVACYBLUR_H
#define PRIVACYBLUR_H

#include <QList>
#include <QString>

class QWebEngineProfile;

// Blurs WhatsApp's content until you hover it, so someone glancing at the
// screen cannot read your chats.
//
// Everything is anchored on structure — #pane-side, #main, role="row" — and not
// on a single one of WhatsApp's class names, which are compiler-generated and
// change with each of its deploys.
namespace PrivacyBlur {

struct Level {
  QString id;
  QString name;
};

QList<Level> levels();

QString currentLevelId();
void setCurrentLevelId(const QString &id);

// Empty when blurring is off.
QString scriptSource();

void install(QWebEngineProfile *profile);

} // namespace PrivacyBlur

#endif // PRIVACYBLUR_H
