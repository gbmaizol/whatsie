#ifndef CHATWALLPAPER_H
#define CHATWALLPAPER_H

#include <QString>

class QWebEngineProfile;

// A custom background image for the chat pane, which WhatsApp Web itself does
// not offer. The image is painted by a style rule on #main — the one stable
// selector in that pane; every class around it is obfuscated and changes.
namespace ChatWallpaper {

// The style rule is carried as a data: URI inside an injected <style>, so it
// survives WhatsApp rebuilding its DOM and needs no file access from the page.
QString scriptSource();

// Register (or drop) the script on the profile, for pages loaded from now on.
void install(QWebEngineProfile *profile);

// Downscale and re-encode the chosen image into the app's data directory, so a
// 12-megapixel photo does not become a multi-megabyte string injected into
// every page load. Returns false and fills `error` when the file is not an
// image we can read.
bool setImage(const QString &sourcePath, QString *error);

void clear();

// Empty when no wallpaper is set.
QString storedImagePath();

} // namespace ChatWallpaper

#endif // CHATWALLPAPER_H
