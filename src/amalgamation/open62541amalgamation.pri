# TODO : help needed for MAC

# Only pass once
!build_pass {
	# Options
	ua_namespace_full {
		UA_NAMESPACE = "-DUA_NAMESPACE_ZERO=FULL"
		message("Enabled ua_namespace_full.")
	}
	ua_encryption {
		# build encryption mbedtls dependency
		include($$PWD/open62541encryption.pri)
		# configure open62541 to use mbedtls
		MBEDTLS_PATH = $$PWD/../../depends/mbedtls.git
		MBEDTLS_INCLUDE_DIRS = "-DMBEDTLS_INCLUDE_DIRS=$${MBEDTLS_PATH}/include"
		win32 {
			CONFIG(debug, debug|release) {
				MBEDTLS_FOLDER_LIBRARY = "-DMBEDTLS_FOLDER_LIBRARY=$${MBEDTLS_PATH}/build/library/Debug"
			} else {
				MBEDTLS_FOLDER_LIBRARY = "-DMBEDTLS_FOLDER_LIBRARY=$${MBEDTLS_PATH}/build/library/Release"
			}	
		}
		linux-g++ {
			MBEDTLS_FOLDER_LIBRARY = "-DMBEDTLS_FOLDER_LIBRARY=$${MBEDTLS_PATH}/build/library"
		}
		UA_ENCRYPTION = "-DUA_ENABLE_ENCRYPTION_MBEDTLS=ON $${MBEDTLS_INCLUDE_DIRS} $${MBEDTLS_FOLDER_LIBRARY}"
		message("Enabled ua_encryption (mbedtls).")
	}
	ua_events {
		UA_NAMESPACE = "-DUA_NAMESPACE_ZERO=FULL"
		UA_EVENTS    = "-DUA_ENABLE_SUBSCRIPTIONS_EVENTS=ON"
		message("Enabled ua_events.")
	}
	ua_alarms_conditions {
		UA_NAMESPACE = "-DUA_NAMESPACE_ZERO=FULL"
		UA_EVENTS    = "-DUA_ENABLE_SUBSCRIPTIONS_EVENTS=ON"
		UA_ALARMS    = "-DUA_ENABLE_SUBSCRIPTIONS_ALARMS_CONDITIONS=ON"
		message("Enabled ua_alarms_conditions.")
	}
	ua_historizing {
		UA_HISTORIZING = "-DUA_ENABLE_HISTORIZING=ON"
		message("Enabled ua_historizing.")
	}
	# Paths
	OPEN62541_PATH       = $$PWD/../../depends/open62541.git
	OPEN62541_BUILD_PATH = $$OPEN62541_PATH/build
	OPEN62541_H_PATH     = $$OPEN62541_BUILD_PATH/open62541.h
	OPEN62541_C_PATH     = $$OPEN62541_BUILD_PATH/open62541.c
	OPEN62541_H_LOCAL    = $$PWD/open62541.h
	OPEN62541_C_LOCAL    = $$PWD/open62541.c
	# Windows
	win32 {
		message("Compiling Open62541 for Windows")
		# Fix PWD slashes
		OPEN62541_PATH_WIN = $$OPEN62541_PATH
		OPEN62541_PATH_WIN ~= s,/,\\,g
		OPEN62541_BUILD_PATH_WIN = $$OPEN62541_BUILD_PATH
		OPEN62541_BUILD_PATH_WIN ~= s,/,\\,g
		# Look for CMake
		CMAKE_BIN = $$system(where cmake)
		isEmpty(CMAKE_BIN) {
			error("CMake not found. Cannot build open62541 amalgamation.")
		}
		else {
			message("CMake found.")
		}

                # Clean up old build if any
                exists($${OPEN62541_BUILD_PATH}) {
                        system("rmdir $${OPEN62541_BUILD_PATH_WIN} /s /q")
                }
                # Create build
                BUILD_CREATED = FALSE
                system("mkdir $${OPEN62541_BUILD_PATH_WIN}"): BUILD_CREATED = TRUE
                equals(BUILD_CREATED, TRUE) {
                        message("Build directory created for open62541.")
                }
                else {
                        error("Build directory could not be created for open62541.")
                }

                contains(QMAKE_CC, cl) {
                        #VisualStudio build
                        message("Compiling open62541 with VisualStudio")

                        # Look for msbuild
                        MSBUILD_BIN = $$system(where msbuild)
                        isEmpty(MSBUILD_BIN) {
                                error("MsBuild not found. Cannot build open62541 amalgamation.")
                        }
                        else {
                                message("MsBuild found.")
                        }

                        # Find compatible compiler
                        MSVC_VER = $$(VisualStudioVersion)
                        equals(MSVC_VER, 12.0){
                                    message("Compiler Detected : MSVC++ 12.0 (Visual Studio 2013)")
                                    COMPILER = "Visual Studio 12 2013"
                        }
                        equals(MSVC_VER, 14.0){
                                    message("Compiler Detected : MSVC++ 14.0 (Visual Studio 2015)")
                                    COMPILER = "Visual Studio 14 2015"
                        }
                        equals(MSVC_VER, 15.0){
                                    message("Compiler Detected : MSVC++ 15.0 (Visual Studio 2017)")
                                    COMPILER = "Visual Studio 15 2017"
                        }
                        equals(MSVC_VER, 16.0){
                                    message("Compiler Detected : MSVC++ 16.0 (Visual Studio 2019)")
                                    COMPILER = "Visual Studio 16 2019"
                        }
                        equals(MSVC_VER, 17.0){
                                    message("Compiler Detected : MSVC++ 17.0 (Visual Studio 2022)")
                                    COMPILER = "Visual Studio 17 2022"
                        }
                        isEmpty(COMPILER) {
                                    error("No compatible compiler found to generate open62541 amalgamation. VisualStudioVersion is $${MSVC_VER}.")
                            }
                        # Find platform
                        contains(QT_ARCH, i386) {
                                message("Platform Detected : 32 bits")
                                PLATFORM = "Win32"
                                MS_PLATFORM = "/p:Platform=Win32"
                        }
                        contains(QT_ARCH, x86_64) {
                                message("Platform Detected : 64 bits")
                                PLATFORM = "x64"
                                MS_PLATFORM = ""
                        }
                        isEmpty(PLATFORM) {
                                error("Non compatible platform $${QT_ARCH} to generate open62541 amalgamation.")
                        }
                        # Generate CMake project
                        PROJECT_CREATED = FALSE
                        system("cmake $${OPEN62541_PATH_WIN} -B$${OPEN62541_BUILD_PATH_WIN} -DUA_ENABLE_AMALGAMATION=ON $${UA_NAMESPACE} $${UA_ENCRYPTION} $${UA_EVENTS} $${UA_ALARMS} $${UA_HISTORIZING} -G \"$${COMPILER}\" -A $${PLATFORM} -T host=x64"): PROJECT_CREATED = TRUE
                        equals(PROJECT_CREATED, TRUE) {
                                message("CMake generate open62541 successful.")
                        }
                        else {
                                error("CMake generate open62541 failed.")
                        }
                        # Create amalgamation sources with MsBuild
                        HEADER_CREATED = FALSE
                        system("msbuild $${OPEN62541_BUILD_PATH_WIN}\open62541-amalgamation-header.vcxproj $${MS_PLATFORM}"): HEADER_CREATED = TRUE
                        equals(HEADER_CREATED, TRUE) {
                                message("Open62541 header open62541.h successful.")
                        }
                        else {
                                error("Open62541 header open62541.h failed.")
                        }
                        SOURCE_CREATED = FALSE
                        system("msbuild $${OPEN62541_BUILD_PATH_WIN}\open62541-amalgamation-source.vcxproj $${MS_PLATFORM}"): SOURCE_CREATED = TRUE
                        equals(SOURCE_CREATED, TRUE) {
                                message("Open62541 source open62541.c successful.")
                        }
                        else {
                                error("Open62541 source open62541.c failed.")
                        }
                }

                contains(QMAKE_CC, gcc) {
                        ##MinGw build
                        message("Compiling open62541 with MinGw")

                        # Generate CMake project
                        PROJECT_CREATED = FALSE
                        system("cmake $${OPEN62541_PATH_WIN} -B$${OPEN62541_BUILD_PATH_WIN} -DUA_ENABLE_AMALGAMATION=ON $${UA_NAMESPACE} $${UA_ENCRYPTION} $${UA_EVENTS} $${UA_ALARMS} $${UA_HISTORIZING} -G \"MinGW Makefiles\""): PROJECT_CREATED = TRUE
                        equals(PROJECT_CREATED, TRUE) {
                                message("CMake generate open62541 successful.")
                        }
                        else {
                                error("CMake generate open62541 failed.")
                        }
                        # Create amalgamation sources with MsBuild
                        HEADER_CREATED = FALSE
                        system("mingw32-make -C $${OPEN62541_BUILD_PATH} open62541-amalgamation-header"): HEADER_CREATED = TRUE
                        equals(HEADER_CREATED, TRUE) {
                                message("Open62541 header open62541.h successful.")
                        }
                        else {
                                error("Open62541 header open62541.h failed.")
                        }
                        SOURCE_CREATED = FALSE
                        system("mingw32-make -C $${OPEN62541_BUILD_PATH} open62541-amalgamation-source"): SOURCE_CREATED = TRUE
                        equals(SOURCE_CREATED, TRUE) {
                                message("Open62541 source open62541.c successful.")
                        }
                        else {
                                error("Open62541 source open62541.c failed.")
                        }
                }

		# Copy amalgamation locally
		# Fix PWD slashes
		OPEN62541_H_PATH_WIN  = $$OPEN62541_H_PATH
		OPEN62541_H_PATH_WIN ~= s,/,\\,g
		OPEN62541_C_PATH_WIN  = $$OPEN62541_C_PATH
		OPEN62541_C_PATH_WIN ~= s,/,\\,g
		OPEN62541_H_LOCAL_WIN  = $$OPEN62541_H_LOCAL
		OPEN62541_H_LOCAL_WIN ~= s,/,\\,g
		OPEN62541_C_LOCAL_WIN  = $$OPEN62541_C_LOCAL
		OPEN62541_C_LOCAL_WIN ~= s,/,\\,g
		# Copy header
		H_COPY = FALSE
		system("copy /y $${OPEN62541_H_PATH_WIN} $${OPEN62541_H_LOCAL_WIN}"): H_COPY = TRUE
		equals(H_COPY, TRUE) {
			message("Open62541 header file copied locally.")
		}
		else {
			error("Failed to copy open62541 header file.")
		}
		# Copy source
		C_COPY = FALSE
		system("copy /y $${OPEN62541_C_PATH_WIN} $${OPEN62541_C_LOCAL_WIN}"): C_COPY = TRUE
		equals(C_COPY, TRUE) {
			message("Open62541 source file copied locally.")
		}
		else {
			error("Failed to copy open62541 source file.")
		}
	}
	# Linux
	linux-g++ {
		message("Compiling Open62541 for Linux.")
		# Look for CMake
		CMAKE_BIN = $$system(which cmake)
		isEmpty(CMAKE_BIN) {
			error("CMake not found. Cannot build open62541 amalgamation.")
		}
		else {
			message("CMake found.")
		}
		# Look for make
		MAKE_BIN = $$system(which make)
		isEmpty(MAKE_BIN) {
			error("Make not found. Cannot build open62541 amalgamation.")
		}
		else {
			message("Make found.")
		}
		# Clean up old build if any
		exists($${OPEN62541_BUILD_PATH}) {
			system("rm -rf $${OPEN62541_BUILD_PATH}")
		}
		# Create build
		BUILD_CREATED = FALSE
		system("mkdir $${OPEN62541_BUILD_PATH}"): BUILD_CREATED = TRUE
		equals(BUILD_CREATED, TRUE) {
			message("Build directory created for open62541.")
		}
		else {
			error("Build directory could not be created for open62541.")
		}
		# Generate CMake project
		PROJECT_CREATED = FALSE
		system("cmake $${OPEN62541_PATH} -B$${OPEN62541_BUILD_PATH} -DUA_ENABLE_AMALGAMATION=ON $${UA_NAMESPACE} $${UA_ENCRYPTION} $${UA_EVENTS} $${UA_ALARMS} $${UA_HISTORIZING}"): PROJECT_CREATED = TRUE
		equals(PROJECT_CREATED, TRUE) {
			message("CMake generate open62541 successful.")
		}
		else {
			error("CMake generate open62541 failed.")
		}
                # Create amalgamation sources with Make
		HEADER_CREATED = FALSE
		system("make -C $${OPEN62541_BUILD_PATH} open62541-amalgamation-header"): HEADER_CREATED = TRUE
		equals(HEADER_CREATED, TRUE) {
			message("Open62541 header open62541.h successful.")
		}
		else {
			error("Open62541 header open62541.h failed.")
		}
		SOURCE_CREATED = FALSE
		system("make -C $${OPEN62541_BUILD_PATH} open62541-amalgamation-source"): SOURCE_CREATED = TRUE
		equals(SOURCE_CREATED, TRUE) {
			message("Open62541 source open62541.c successful.")
		}
		else {
			error("Open62541 source open62541.c failed.")
		}
		# Copy header
		H_COPY = FALSE
		system("yes | cp -rf $${OPEN62541_H_PATH} $${OPEN62541_H_LOCAL}"): H_COPY = TRUE
		equals(H_COPY, TRUE) {
			message("Open62541 header file copied locally.")
		}
		else {
			error("Failed to copy open62541 header file.")
		}
		# Copy source
		C_COPY = FALSE
		system("yes | cp -rf $${OPEN62541_C_PATH} $${OPEN62541_C_LOCAL}"): C_COPY = TRUE
		equals(C_COPY, TRUE) {
			message("Open62541 source file copied locally.")
		}
		else {
			error("Failed to copy open62541 source file.")
		}
	}
	# Mac OS
	mac {
		#message("Compiling Open62541 for Mac.")
		if(!exists($${OPEN62541_H_LOCAL}) || !exists($${OPEN62541_C_LOCAL})) {
			error("Automatic QMake build for open62541 amalgamation not yet supported on Mac. Please build it manually and move the files here.")
		}
	}	
} # Only pass once
