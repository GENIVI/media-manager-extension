 /**
  * Copyright (C) 2014, Jaguar Land Rover
  *
  * Author: Jonatan Palsson <jonatan.palsson@pelagicore.com>
  *
  * This file is part of the GENIVI Media Manager Proof-of-Concept
  * For further information, see http://genivi.org/
  *
  * This Source Code Form is subject to the terms of the Mozilla Public
  * License, v. 2.0. If a copy of the MPL was not distributed with this
  * file, You can obtain one at http://mozilla.org/MPL/2.0/.
  */

#include "string.h"

#include "jsonrpc.h"

#include "rpc.h"
#include "capi-client-indexer.h"
#include "capi-client-browser.h"
#include "capi-client-player.h"
#include "raw_message_interface.h"
#include "glib.h"


static struct jsonrpc_method_entry_t method_table[] = {
    /* Indexer API */
    { "getDatabasePath",  capi_client_indexer_getDatabasePath,  "" },
    { "startIndexing",    capi_client_indexer_startIndexing,    "" },
    { "stopIndexing",     capi_client_indexer_stopIndexing,     "" },
    { "getIndexerStatus", capi_client_indexer_getIndexerStatus, "" },

    /* Browser API */
    { "listContainers", capi_client_browser_listContainers, "o"},
    { "listContainersEx", capi_client_browser_listContainersEx, "o"},
    { "listItems", capi_client_browser_listItems, "o"},
    { "listItemsEx", capi_client_browser_listItemsEx, "o"},
    { "createReference", capi_client_browser_createReference, "o"},
    { "createContainer", capi_client_browser_createContainer, "o"},
    { "discoverMediaManagers", capi_client_browser_discoverMediaManagers, ""},
    { "listChildren", capi_client_browser_listChildren, "o"},
    { "listChildrenEx", capi_client_browser_listChildrenEx, "o"},
    { "searchObjects", capi_client_browser_searchObjects, "o"},
    { "searchObjectsEx", capi_client_browser_searchObjectsEx, "o"},
    { "listIndexes", capi_client_browser_listIndexes, "o"},

    /* Player API */
    { "openUri", capi_client_player_openUri, "o"},
    { "pause", capi_client_player_pause, "o"},
    { "play", capi_client_player_play, "o"},
    { "stop", capi_client_player_stop, "o"},
    { "playPause", capi_client_player_playPause, "o"},
    { "next", capi_client_player_next, "o"},
    { "openPlaylist", capi_client_player_openPlaylist, "o"},
    { "previous", capi_client_player_previous, "o"},
    { "seek", capi_client_player_seek, "o"},
    { "setPosition", capi_client_player_setPosition, "o"},
    { "setMuted", capi_client_player_setMuted, "o"},
    { "getMuted", capi_client_player_getMuted, "o"},
    { "setShuffled", capi_client_player_setShuffled, "o"},
    { "getShuffled", capi_client_player_getShuffled, "o"},
    { "setRepeated", capi_client_player_setRepeated, "o"},
    { "getRepeated", capi_client_player_getRepeated, "o"},
    { "setRate", capi_client_player_setRate, "o"},
    { "getRate", capi_client_player_getRate, "o"},
    { "getVolume", capi_client_player_getVolume, "o"},
    { "setVolume", capi_client_player_setVolume, "o"},
    { "getCanGoNext", capi_client_player_getCanGoNext, "o"},
    { "getCanGoPrevious", capi_client_player_getCanGoPrevious, "o"},
    { "getCanPause", capi_client_player_getCanPause, "o"},
    { "getCanPlay", capi_client_player_getCanPlay, "o"},
    { "getCanSeek", capi_client_player_getCanSeek, "o"},
    { "getCurrentTrack", capi_client_player_getCurrentTrack, "o"},
    { "getPlaybackStatus", capi_client_player_getPlaybackStatus, "o"},
    { "getPosition", capi_client_player_getPosition, "o"},
    { "getCurrentPlayQueue", capi_client_player_getCurrentPlayQueue, "o"},
    { "dequeueIndex", capi_client_player_dequeueIndex, "o"},
    { "dequeueAll", capi_client_player_dequeueAll, "o"},
    { "enqueueUri", capi_client_player_enqueueUri, "o"},
    { NULL },
};

char *rpc_handle_message (XW_Instance instance, const char *message) {
    capi_client_player_set_xwalk_instance (instance);

    return jsonrpc_handler(message,
                               strlen(message),
                               method_table,
                               NULL);
}

void rpc_send_notification (XW_Instance instance, const char *method, const char *params) {
    char *message = g_strdup_printf("{\"jsonrpc\": \"2.0\", \
                                      \"method\": \"%s\", \
                                      \"params\": %s}",
                                      method,
                                      params);
    sendRawMessage (instance, message);

    g_free (message);
}
