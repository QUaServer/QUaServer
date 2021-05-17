# include pre-compiled open62541 library as a dependency
LIBS += -L$$PWD/../../build/
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
# include header to project
HEADERS += $$PWD/open62541.h

ua_encryption {
    MBEDTLS_PATH = $$PWD/../../depends/mbedtls.git
    # include mbedtls header directory
    INCLUDEPATH += $$MBEDTLS_PATH/build/include
    # include pre-compiled mbedtls library as a dependency
    CONFIG(debug, debug|release) {
        win32 {
            LIBS += -L$$MBEDTLS_PATH/build/library/Debug
        }
        linux-g++ {
            LIBS += -L$$MBEDTLS_PATH/build/library
        }
    } else {
        win32 {
            LIBS += -L$$MBEDTLS_PATH/build/library/Release
        }
        linux-g++ {
            LIBS += -L$$MBEDTLS_PATH/build/library
        }
    }   
    LIBS += -lmbedcrypto -lmbedtls -lmbedx509
}
