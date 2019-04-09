#ifndef MYEVENTTYPE_H
#define MYEVENTTYPE_H

#include <QUaBaseEvent>

class MyEventType : public QUaBaseEvent
{
    Q_OBJECT

public:
	Q_INVOKABLE explicit MyEventType(QUaServer *server);


	
};

#endif // MYEVENTTYPE_H

