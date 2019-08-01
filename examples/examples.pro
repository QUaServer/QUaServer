TEMPLATE = subdirs
CONFIG  += debug_and_release build_all
# names
SUBDIRS = \
open62541 \
01_basics \
02_methods \
03_references \
04_types \
05_server \
06_users \
07_encryption \
08_events
# directories
open62541.subdir     = $$PWD/../src/amalgamation/open62541.pro
01_basics.subdir     = $$PWD/01_basics/01_basics.pro
02_methods.subdir    = $$PWD/02_methods/02_methods.pro
03_references.subdir = $$PWD/03_references/03_references.pro
04_types.subdir      = $$PWD/04_types/04_types.pro
05_server.subdir     = $$PWD/05_server/05_server.pro
06_users.subdir      = $$PWD/06_users/06_users.pro
07_encryption.subdir = $$PWD/07_encryption/07_encryption.pro
08_events.subdir     = $$PWD/08_events/08_events.pro
# dependencies
01_basics.depends     = open62541
02_methods.depends    = open62541
03_references.depends = open62541
04_types.depends      = open62541
05_server.depends     = open62541
06_users.depends      = open62541
07_encryption.depends = open62541
08_events.depends     = open62541








