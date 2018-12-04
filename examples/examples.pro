TEMPLATE = subdirs

SUBDIRS += \
$$PWD/../open62541.pro \
$$PWD/01_tutorial_datatypes/tutorial_datatypes.pro \
$$PWD/02_tutorial_server_firststeps/tutorial_server_firststeps.pro \
$$PWD/03_tutorial_server_variable/tutorial_server_variable.pro \
$$PWD/04_tutorial_server_datasource/tutorial_server_datasource.pro \
$$PWD/05_tutorial_server_variabletype/tutorial_server_variabletype.pro \
$$PWD/06_tutorial_server_object/tutorial_server_object.pro \
$$PWD/07_tutorial_server_method/tutorial_server_method.pro \
$$PWD/09_tutorial_client_firststeps/tutorial_client_firststeps.pro \
# Events not yet working on v0.3-rc4
#$$PWD/08_tutorial_server_events/tutorial_server_events.pro \