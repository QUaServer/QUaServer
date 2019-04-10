TEMPLATE = subdirs

CONFIG += debug_and_release build_all

SUBDIRS = \
$$PWD/../src/amalgamation/open62541.pro \
$$PWD/01_basics/01_basics.pro \
$$PWD/02_methods/02_methods.pro \
$$PWD/03_references/03_references.pro \
$$PWD/04_types/04_types.pro \

