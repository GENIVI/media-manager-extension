# Copyright (C) 2014, Jaguar Land Rover
#
# Author: Jonatan Palsson <jonatan.palsson@pelagicore.com>
#
# This file is part of the GENIVI Media Manager Proof-of-Concept
# For further information, see http://genivi.org/
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

INCLUDE (${MEDIAMANAGER_SOURCE_DIR}/shared.cmake)

SET (SOURCES
    mediamanager-extension.c
    ${JSONRPC_DIR}/jsonrpc.c
    rpc.c
    capi-client-indexer.cpp
    capi-client-browser.cpp
    capi-client-player.cpp)

SET (HEADERS
   ${MEDIAMANAGER_BINARY_DIR}/generated/mediamanager-extension.h
   ${JSONRPC_DIR}/jsonrpc.h
   rpc.h
   capi-client-indexer.h
   capi-client-browser.h
   capi-client-player.h)

MESSAGE(FATAL ${SOURCES})

ADD_LIBRARY (mediamanager SHARED ${SOURCES}
                                 ${HEADERS}
                                 ${GENERATED_SOURCES}
                                 ${GENERATED_HEADERS})

# Destination is relative to prefix
INSTALL (TARGETS mediamanager
         DESTINATION lib/tizen-extensions-crosswalk)

TARGET_LINK_LIBRARIES (mediamanager ${PC_JANSSON_LIBRARIES})
TARGET_LINK_LIBRARIES (mediamanager ${PC_GLIB_LIBRARIES})
TARGET_LINK_LIBRARIES (mediamanager ${PC_COMMONAPI_LIBRARIES})
TARGET_LINK_LIBRARIES (mediamanager ${PC_COMMONAPIDBUS_LIBRARIES})
