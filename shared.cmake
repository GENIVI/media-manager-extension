# Location for the JSON-RPC library
#SET (JSONRPC_DIR ${MEDIAMANAGER_BINARY_DIR}/external/jsonrpc/)
SET (JSONRPC_DIR ${MEDIAMANAGER_SOURCE_DIR}/external/jsonrpc/)

# Known good checkout of JSON-RPC library
SET (JSONRPC_CHEKOUT 56ce5385cade24adb2151e1ab2a2dee7ce3e95d4)

SET (INTERFACE_DIR ${MEDIAMANAGER_SOURCE_DIR}/interfaces/src-gen/)

SET (CUSTOM_DBUS_PATH /usr/local/lib/)

FIND_PACKAGE (PkgConfig)

# Add libjansson -I and -l flags
pkg_check_modules (PC_GLIB REQUIRED glib-2.0)
pkg_check_modules (PC_JANSSON REQUIRED jansson)
pkg_check_modules (PC_COMMONAPI REQUIRED CommonAPI)
pkg_check_modules (PC_COMMONAPIDBUS REQUIRED CommonAPI-DBus)

INCLUDE_DIRECTORIES (${PC_COMMONAPI_INCLUDE_DIRS}
                     ${PC_COMMONAPIDBUS_INCLUDE_DIRS}
                     ${PC_JANSSON_INCLUDE_DIRS}
                     ${PC_GLIB_INCLUDE_DIRS}
                     )

INCLUDE_DIRECTORIES (${MEDIAMANAGER_SOURCE_DIR}/common
                     ${MEDIAMANAGER_BINARY_DIR}/generated
                     ${MEDIAMANAGER_SOURCE_DIR}/interfaces/src-gen/
                     ${JSONRPC_DIR})

INCLUDE_DIRECTORIES (${INTERFACE_DIR})

LINK_DIRECTORIES (${CUSTOM_DBUS_PATH})
LINK_DIRECTORIES (${PC_COMMONAPIDBUS_LIBRARY_DIRS})

ADD_DEFINITIONS (${PC_COMMONAPIDBUS_CFLAGS})

SET(CMAKE_CXX_FLAGS "-std=c++0x")

# Generate C headers containing JavaScript sources.
# These go in to generated/ in the build directory
ADD_CUSTOM_COMMAND (OUTPUT ${MEDIAMANAGER_BINARY_DIR}/generated/mediamanager-extension.h
                    DEPENDS ${MEDIAMANAGER_SOURCE_DIR}/extension/api.js
                    COMMAND tools/js2c.sh    #Generator script
                            extension/api.js # Source
                            ${MEDIAMANAGER_BINARY_DIR}/generated/mediamanager-extension.h # Dest
                    WORKING_DIRECTORY ${MEDIAMANAGER_SOURCE_DIR})

# Command used to check out JSON-RPC sources, used unless user has supplied
# these files on her own
#ADD_CUSTOM_COMMAND (OUTPUT ${JSONRPC_DIR}/jsonrpc.c
#                           ${JSONRPC_DIR}/jsonrpc.h
#                    COMMAND mkdir -p ${JSONRPC_DIR} &&
#                            git clone https://github.com/pijyoi/jsonrpc.git ${JSONRPC_DIR} &&
#                            cd ${JSONRPC_DIR} &&
#                            git checkout ${JSONRPC_CHECKOUT})

FILE (GLOB GENERATED_SOURCES
     ${INTERFACE_DIR}/org/genivi/mediamanager/*.cpp)

FILE (GLOB GENERATED_HEADERS
     ${INTERFACE_DIR}/org/genivi/mediamanager/*.h)

ADD_CUSTOM_COMMAND (OUTPUT ${GENERATED_SOURCES} ${GENERATED_HEADERS}
                    COMMAND git submodule init &&
                            git submodule update)

