TEMPLATE = subdirs
CONFIG  += debug_and_release build_all
# NOTE : had to move this pro file to repo's root directory because qmake does not work well on Linux when a subdir is out of $$PWD
# names
SUBDIRS += \
00_amalgamation \
01_basics \
02_methods \
03_references \
04_types \
05_server \
06_users \
07_encryption \
08_events \
09_serialization
# directories
00_amalgamation.subdir  = $$PWD/src/amalgamation
01_basics.subdir        = $$PWD/examples/01_basics
02_methods.subdir       = $$PWD/examples/02_methods
03_references.subdir    = $$PWD/examples/03_references
04_types.subdir         = $$PWD/examples/04_types
05_server.subdir        = $$PWD/examples/05_server
06_users.subdir         = $$PWD/examples/06_users
07_encryption.subdir    = $$PWD/examples/07_encryption
08_events.subdir        = $$PWD/examples/08_events
09_serialization.subdir = $$PWD/examples/09_serialization
# dependencies
00_amalgamation.depends  =
01_basics.depends        = 00_amalgamation
02_methods.depends       = 00_amalgamation
03_references.depends    = 00_amalgamation
04_types.depends         = 00_amalgamation
05_server.depends        = 00_amalgamation
06_users.depends         = 00_amalgamation
07_encryption.depends    = 00_amalgamation
08_events.depends        = 00_amalgamation
09_serialization.depends = 00_amalgamation