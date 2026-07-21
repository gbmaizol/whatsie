#ifndef HDMEDIA_H
#define HDMEDIA_H

#include <QString>

class QWebEngineProfile;

// "Send photos and videos in HD by default."
//
// WhatsApp Web uploads media at Standard quality unless you tap the HD control
// in the media editor for each attachment. This injects a MutationObserver that,
// when the editor's HD control appears and HD is not already chosen, selects it —
// so attachments default to HD.
//
// There is no stable/public API for this, so the observer matches the editor's
// HD control by the attributes WhatsApp currently uses and is written entirely
// defensively (a try/catch that can never throw into the page). It therefore
// tracks WhatsApp Web's DOM and may need updating if that markup changes.
namespace HdMedia {

bool isEnabled();
void setEnabled(bool enabled);

// The injected script (installs/removes the observer per the setting).
QString scriptSource();

// (Re)install on a profile; removes the script when disabled.
void install(QWebEngineProfile *profile);

} // namespace HdMedia

#endif // HDMEDIA_H
