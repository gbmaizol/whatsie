#ifndef STORAGEINFO_H
#define STORAGEINFO_H

#include <QString>

// Small helpers for the storage manager: the on-disk size of a directory and a
// human-readable byte formatter. Both are pure and unit-tested.
namespace StorageInfo {

// Total size in bytes of everything under `path` (recursive). 0 if it does not
// exist or is empty.
qint64 directorySize(const QString &path);

// Format a byte count as "1.5 MB", "820 KB", "0 B", … (binary units, 1 KB=1024).
QString humanReadable(qint64 bytes);

} // namespace StorageInfo

#endif // STORAGEINFO_H
