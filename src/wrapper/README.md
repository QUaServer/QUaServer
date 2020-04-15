# Implemented 62541 Types

| C++ Type                       | Spec              | UA Type Hierarchy  |
|--------------------------------|-------------------|--------------------|
| QUaBaseVariable                | Part 5 - 7.2      | `BaseVariableType` |
| QUaProperty                    | Part 5 - 7.3      | `BaseVariableType/PropertyType` |
| QUaBaseDataVariable            | Part 5 - 7.4      | `BaseVariableType/BaseDataVariableType` |
| QUaStateVariable               | Part 5 - B.4.3    | `BaseVariableType/BaseDataVariableType/StateVariableType` |
| QUaFiniteStateVariable         | Part 5 - B.4.6    | `BaseVariableType/BaseDataVariableType/StateVariableType/FiniteStateVariableType` |
| QUaTwoStateVariable            | Part 9 - 5.2      | `BaseVariableType/BaseDataVariableType/StateVariableType/TwoStateVariableType` |
| QUaConditionVariable           | Part 9 - 5.3      | `BaseVariableType/BaseDataVariableType/ConditionVariableType` |
| QUaTransitionVariable          | Part 5 - B.4.4    | `BaseVariableType/BaseDataVariableType/TransitionVariableType` |
| QUaFiniteTransitionVariable    | Part 5 - B.4.7    | `BaseVariableType/BaseDataVariableType/TransitionVariableType/FiniteTransitionVariableType` |
| QUaBaseObject                  | Part 5 - 6.2      | `BaseObjectType` |
| QUaFolderObject                | Part 5 - 6.6      | `BaseObjectType/FolderType` |
| QUaStateMachine                | Part 5 - B.4.2    | `BaseObjectType/StateMachineType` |
| QUaFiniteStateMachine          | Part 5 - B.4.5    | `BaseObjectType/StateMachineType/FiniteStateMachineType` |
| QUaState                       | Part 5 - B.4.8    | `BaseObjectType/StateType` |
| QUaTransition                  | Part 5 - B.4.10   | `BaseObjectType/TransitionType` |
| QUaExclusiveLimitStateMachine  | Part 9 - 5.8.12.2 | `BaseObjectType/StateMachineType/FiniteStateMachineType/ExclusiveLimitStateMachineType` |
| QUaBaseEvent                   | Part 5 - 6.4.2    | `BaseObjectType/BaseEventType` |
| QUaBaseModelChangeEvent        | Part 5 - 6.4.31   | `BaseObjectType/BaseEventType/BaseModelChangeEventType` |
| QUaGeneralModelChangeEvent     | Part 5 - 6.4.32   | `BaseObjectType/BaseEventType/BaseModelChangeEventType/GeneralModelChangeEvent` |
| QUaSystemEvent                 | Part 5 - 6.4.28   | `BaseObjectType/BaseEventType/SystemEventType` |
| QUaRefreshStartEvent           | Part 9 - 5.11.2   | `BaseObjectType/BaseEventType/SystemEventType/RefreshStartEventType` |
| QUaRefreshEndEvent             | Part 9 - 5.11.3   | `BaseObjectType/BaseEventType/SystemEventType/RefreshEndEventType` |
| QUaTransitionEvent             | Part 5 - B.4.16   | `BaseObjectType/BaseEventType/TransitionEventType` |
| QUaCondition                   | Part 9 - 5.5      | `BaseObjectType/BaseEventType/ConditionType` |
| QUaAcknowledgeableCondition    | Part 9 - 5.7.2    | `BaseObjectType/BaseEventType/ConditionType/AcknowledgeableConditionType` |
| QUaAlarmCondition              | Part 9 - 5.8.2    | `BaseObjectType/BaseEventType/ConditionType/AcknowledgeableConditionType/AlarmConditionType` |
| QUaDiscreteAlarm               | Part 9 - 5.8.17.1 | `BaseObjectType/BaseEventType/ConditionType/AcknowledgeableConditionType/AlarmConditionType/DiscreteAlarmType` |
| QUaOffNormalAlarm              | Part 9 - 5.8.17.2 | `BaseObjectType/BaseEventType/ConditionType/AcknowledgeableConditionType/AlarmConditionType/DiscreteAlarmType/OffNormalAlarmType` |
| QUaLimitAlarm                  | Part 9 - 5.8.11   | `BaseObjectType/BaseEventType/ConditionType/AcknowledgeableConditionType/AlarmConditionType/LimitAlarmType` |
| QUaExclusiveLimitAlarm         | Part 9 - 5.8.12.3 | `BaseObjectType/BaseEventType/ConditionType/AcknowledgeableConditionType/AlarmConditionType/LimitAlarmType/ExclusiveLimitAlarmType` |
| QUaExclusiveLevelAlarm         | Part 9 - 5.8.14.3 | `BaseObjectType/BaseEventType/ConditionType/AcknowledgeableConditionType/AlarmConditionType/LimitAlarmType/ExclusiveLimitAlarmType/ExclusiveLevelAlarmType` |







