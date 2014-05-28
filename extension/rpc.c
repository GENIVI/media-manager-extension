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


static struct jsonrpc_method_entry_t method_table[] = {
    /* Indexer API */
    { "getDatabasePath",  capi_client_indexer_getDatabasePath,  "" },
    { "startIndexing",    capi_client_indexer_startIndexing,    "" },
    { "stopIndexing",     capi_client_indexer_stopIndexing,     "" },
    { "getIndexerStatus", capi_client_indexer_getIndexerStatus, "" },

    /* Browser API */
    { "listContainers", capi_client_browser_listContainers, "o"},
    /* Player API */
    { NULL },
};

char *rpc_handle_message (const char *message) {
    return jsonrpc_handler(message,
                               strlen(message),
                               method_table,
                               NULL);
}
