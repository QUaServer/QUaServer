#ifndef QUAENUM_H
#define QUAENUM_H

typedef qint64 QUaEnumKey;
struct QUaEnumEntry
{
    QUaLocalizedText displayName;
    QUaLocalizedText description;
};
Q_DECLARE_METATYPE(QUaEnumEntry);
inline bool operator==(const QUaEnumEntry& lhs, const QUaEnumEntry& rhs)
{
    return lhs.displayName == rhs.displayName && lhs.description == rhs.description;
}
typedef QMap<QUaEnumKey, QUaEnumEntry> QUaEnumMap;
typedef QMapIterator<QUaEnumKey, QUaEnumEntry> QUaEnumMapIter;

#endif // QUAENUM_H
