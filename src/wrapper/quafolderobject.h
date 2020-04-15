#ifndef QUAFOLDEROBJECT_H
#define QUAFOLDEROBJECT_H

#include <QUaBaseObject>

class QUaFolderObject : public QUaBaseObject
{
    Q_OBJECT

friend class QUaServer;

public:
	Q_INVOKABLE explicit QUaFolderObject(QUaServer *server);

	

};

#endif // QUAFOLDEROBJECT_H