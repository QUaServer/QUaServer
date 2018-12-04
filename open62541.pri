# include pre-compiled library as a dependency
LIBS += -L$$PWD/build/
CONFIG(debug, debug|release) {
	LIBS += -lopen62541d
} else {
	LIBS += -lopen62541
}	
# include sockets dependency for windows
win32 {
	LIBS += -lws2_32 
}
# include header directory
INCLUDEPATH += $$PWD/src