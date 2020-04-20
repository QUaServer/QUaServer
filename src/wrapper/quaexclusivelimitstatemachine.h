#ifndef QUAEXCLUSIVELIMITSTATEMACHINE_H
#define QUAEXCLUSIVELIMITSTATEMACHINE_H

#include <QUaFiniteStateMachine>

#ifdef UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

class QUaExclusiveLimitStateMachine : public QUaFiniteStateMachine
{
    Q_OBJECT

	Q_PROPERTY(QUaExclusiveLimitState exclusiveLimitState READ exclusiveLimitState WRITE setExclusiveLimitState NOTIFY exclusiveLimitStateChanged)

public:
	Q_INVOKABLE explicit QUaExclusiveLimitStateMachine(
		QUaServer* server
	);


	// NOTE : the states available for this state machine only
	//        exist on its type. The instance only holds the
	//        current state and last transtion

	// the current limit state that the owning alarm is on currently
	// a value of None or Normal corresponds to a null state in the state machine
	QUaExclusiveLimitState exclusiveLimitState() const;
	void setExclusiveLimitState(const QUaExclusiveLimitState& exclusiveLimitState);

	QUaExclusiveLimitTransition lastExclusiveLimitTransition() const;
	void setLastExclusiveLimitTransition(const QUaExclusiveLimitTransition& lastExclusiveLimitTransition);

	// adds highhigh to available states
	void setHighHighLimitRequired(const bool& highHighLimitRequired);
	// adds high to available states
	void setHighLimitRequired    (const bool& highLimitRequired    );
	// adds low to available states
	void setLowLimitRequired     (const bool& lowLimitRequired     );
	// adds lowlow to available states
	void setLowLowLimitRequired  (const bool& lowLowLimitRequired  );

signals:
	void exclusiveLimitStateChanged(const QUaExclusiveLimitState& state);

protected:

	// get nodeIds of available states and transtions (only exist on type)

	static QUaNodeId stateHighHighNodeId();
	static QUaNodeId stateHighNodeId();
	static QUaNodeId stateLowNodeId();
	static QUaNodeId stateLowLowNodeId();

	static QUaNodeId transitionLowToLowLowNodeId();
	static QUaNodeId transitionLowLowToLowNodeId();
	static QUaNodeId transitionHighToHighHighNodeId();
	static QUaNodeId transitionHighHighToHighNodeId();
};

#endif // UA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS

#endif // QUAEXCLUSIVELIMITSTATEMACHINE_H

