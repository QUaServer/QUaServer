# include pre-compiled open62541 library as a dependency
LIBS += -L$$PWD/../build/
CONFIG(debug, debug|release) {
	LIBS += -lopen62541d
} else {
	LIBS += -lopen62541
}	
# include sockets dependency for windows
win32 {
	LIBS += -lws2_32 
}
# include open62541 header directory
INCLUDEPATH += $$PWD/

include($$PWD/open62541opts.pri)

# [ENCRYPTION]
equals(USE_ENCRYPTION, true) {
	# include mbedtls header directory
	INCLUDEPATH += $$PWD/../../mbedtls.git/build/include
	# include pre-compiled mbedtls library as a dependency
	CONFIG(debug, debug|release) {
		LIBS += -L$$PWD/../../mbedtls.git/build/library/Debug
	} else {
		LIBS += -L$$PWD/../../mbedtls.git/build/library/Release
	}	
	LIBS += -lmbedcrypto -lmbedtls -lmbedx509
}
