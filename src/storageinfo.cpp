#include "storageinfo.h"

#include <QDirIterator>
#include <QFileInfo>

namespace StorageInfo {

qint64 directorySize(const QString &path) {
  qint64 total = 0;
  QDirIterator it(path, QDir::Files | QDir::NoSymLinks | QDir::Hidden,
                  QDirIterator::Subdirectories);
  while (it.hasNext()) {
    it.next();
    total += it.fileInfo().size();
  }
  return total;
}

QString humanReadable(qint64 bytes) {
  if (bytes < 0)
    bytes = 0;
  static const char *units[] = {"B", "KB", "MB", "GB", "TB"};
  double v = double(bytes);
  int u = 0;
  while (v >= 1024.0 && u < 4) {
    v /= 1024.0;
    ++u;
  }
  // Whole number for bytes; one decimal for KB and up.
  if (u == 0)
    return QString::number(bytes) + QLatin1String(" B");
  return QString::number(v, 'f', 1) + QLatin1Char(' ') +
         QLatin1String(units[u]);
}

} // namespace StorageInfo
