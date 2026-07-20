#ifndef QUICKREPLY_H
#define QUICKREPLY_H

#include <QString>

// "Click the notification and just start typing."
//
// Activating a desktop notification already raises the window and opens the
// chat; this puts the caret in WhatsApp Web's message box as well, so a reply
// needs no extra click. The injected snippet is defensive: WhatsApp Web's DOM
// changes often, so it tries several selectors and never throws.
//
// A true inline-reply field inside the notification is deliberately not used:
// the XDG portal notification spec has no standard text-entry field, and the
// freedesktop "inline-reply" capability is not available on every backend.
namespace QuickReply {

// JavaScript that focuses the message composer, if one is present.
QString focusComposerScript();

} // namespace QuickReply

#endif // QUICKREPLY_H
