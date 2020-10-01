#ifndef QUAOPTIONSET_H
#define QUAOPTIONSET_H

// NOTE : support up to 64-bit option sets (128bits total = 64 for value, 64 for validity)
typedef quint64 QUaOptionSetBit;

typedef QMap<QUaOptionSetBit, QUaLocalizedText> QUaOptionSetMap;
typedef QMapIterator<QUaOptionSetBit, QUaLocalizedText> QUaOptionSetMapIter;

// User validation
typedef std::function<bool(const QString&, const QString&)> QUaValidationCallback;

#endif // QUAOPTIONSET_H
