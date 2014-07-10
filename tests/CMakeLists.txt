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
     ${JSONRPC_DIR}/jsonrpc.c
     ${MEDIAMANAGER_SOURCE_DIR}/extension/rpc.c
     ${MEDIAMANAGER_SOURCE_DIR}/extension/capi-client-indexer.cpp
     ${MEDIAMANAGER_SOURCE_DIR}/extension/capi-client-browser.cpp
     ${MEDIAMANAGER_SOURCE_DIR}/extension/capi-client-player.cpp
     ${MEDIAMANAGER_SOURCE_DIR}/extension/mediamanager-extension.c
     extension_test.c
     )

SET (HEADERS
     ${JSONRPC_DIR}/jsonrpc.h
     ${MEDIAMANAGER_SOURCE_DIR}/extension/rpc.h
     ${MEDIAMANAGER_SOURCE_DIR}/extension/capi-client-indexer.h
     ${MEDIAMANAGER_SOURCE_DIR}/extension/capi-client-browser.h
     ${MEDIAMANAGER_SOURCE_DIR}/extension/capi-client-player.h
     ${MEDIAMANAGER_BINARY_DIR}/generated/mediamanager-extension.h
     )

INCLUDE_DIRECTORIES (${MEDIAMANAGER_SOURCE_DIR}/extension/)

ADD_EXECUTABLE (mediamanager-test ${SOURCES}
                                  ${HEADERS}
                                  ${GENERATED_SOURCES}
                                  ${GENERATED_HEADERS})

TARGET_LINK_LIBRARIES (mediamanager-test ${PC_JANSSON_LIBRARIES})
TARGET_LINK_LIBRARIES (mediamanager-test ${PC_GLIB_LIBRARIES})
TARGET_LINK_LIBRARIES (mediamanager-test ${PC_COMMONAPI_LIBRARIES})
TARGET_LINK_LIBRARIES (mediamanager-test ${PC_COMMONAPIDBUS_LIBRARIES})
