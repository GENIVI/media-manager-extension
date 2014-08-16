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

#include <glib.h>

#include "rpc.h"
#include "capi-client-indexer.h"

static void test_can_do_capi_call () {
    char *reply = rpc_handle_message (0,
                                      "{ \"jsonrpc\": \"2.0\","
                                      "  \"method\":  \"GetDatabasePath\","
                                      "  \"id\":      \"1\" }");
    g_print (reply);
    g_assert (reply != NULL);

    g_assert(g_strrstr(reply, "\"error\"") == -1);
}

int main (int argc, char **argv) {
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/commonapi/simple-message", test_can_do_capi_call);

    return g_test_run ();
}
