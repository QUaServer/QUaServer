TEMPLATE = subdirs
CONFIG  += debug_and_release build_all
# TODO : still cannot make amalgamation project work on linux
# names
win32 {
    SUBDIRS += \
    00_amalgamation
}
SUBDIRS += \
01_basics \
02_methods \
03_references \
04_types \
05_server \
06_users \
07_encryption \
08_events
# directories
win32 {
    00_amalgamation.subdir = $$PWD/../src/amalgamation
}
01_basics.subdir       = $$PWD/01_basics
02_methods.subdir      = $$PWD/02_methods
03_references.subdir   = $$PWD/03_references
04_types.subdir        = $$PWD/04_types
05_server.subdir       = $$PWD/05_server
06_users.subdir        = $$PWD/06_users
07_encryption.subdir   = $$PWD/07_encryption
08_events.subdir       = $$PWD/08_events

win32 {
    ## dependencies
    01_basics.depends     = 00_amalgamation
    02_methods.depends    = 00_amalgamation
    03_references.depends = 00_amalgamation
    04_types.depends      = 00_amalgamation
    05_server.depends     = 00_amalgamation
    06_users.depends      = 00_amalgamation
    07_encryption.depends = 00_amalgamation
    08_events.depends     = 00_amalgamation
}
