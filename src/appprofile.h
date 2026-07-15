#ifndef APPPROFILE_H
#define APPPROFILE_H

#include <QString>

// Which account this process is running as, chosen once by --profile=<name> at
// startup. It decides three things that must all agree and must all be known
// before Qt is up: the single-instance key (so two accounts are two running
// apps, not one), the settings file, and the WebEngine storage directory.
//
// The default account — no flag, or --profile=default — keeps the exact paths
// and settings the app has always used, so an upgrade neither moves anyone's
// data nor logs anyone out.
namespace AppProfile {

// Reads --profile=<name>/-p <name> straight out of argv, before QApplication
// exists (QCommandLineParser needs one, and this is needed earlier). An unknown
// or empty name resolves to the default account. Call once, first thing in
// main().
void initFromArgs(int argc, char *argv[]);

// "" for the default account, otherwise a filesystem-safe slug.
QString id();

bool isDefault();

// A suffix for file/instance names: "" for the default account, "-work" for
// --profile=work.
QString suffix();

// For window titles and the tray tooltip: "" for the default account,
// " (work)" for --profile=work, so two accounts can be told apart in the
// taskbar.
QString label();

} // namespace AppProfile

#endif // APPPROFILE_H
