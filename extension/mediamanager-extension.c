 /*
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

/**
 * @file
 * This module is the main entry point from CrossWalk in to the extension code.
 * This module is responsible for opening message channels to and from
 * CrossWalk in to the extension.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "XW_Extension.h"
#include "XW_Extension_SyncMessage.h"

#include "rpc.h"

#include "mediamanager-extension.h"
#include "raw_message_interface.h"

static XW_Extension g_extension = 0;
static const XW_CoreInterface* g_core = NULL;
static const XW_MessagingInterface* g_messaging = NULL;
static const XW_Internal_SyncMessagingInterface* g_sync_messaging = NULL;

static void instance_created(XW_Instance instance) {
    printf("Instance %d created!\n", instance);
}

static void instance_destroyed(XW_Instance instance) {
    printf("Instance %d destroyed!\n", instance);
}

static char* build_response(XW_Instance instance, const char* message) {
    char *ret = rpc_handle_message(instance, message);
    if (!ret)
        return (strdup ("Invalid request!"));
    else
        return ret;
}

static void handle_message(XW_Instance instance, const char* message) {
    char* response = build_response(instance, message);
    g_messaging->PostMessage(instance, response);
    free(response);
}

static void handle_sync_message(XW_Instance instance, const char* message) {
    char* response = build_response(instance, message);
    g_sync_messaging->SetSyncReply(instance, response);
    free(response);
}

static void shutdown(XW_Extension extension) {
    printf("Shutdown\n");
}

void sendRawMessage(XW_Instance instance, const char *message) {
    g_messaging->PostMessage(instance, message);
}

/**
 * Initialize the CrossWalk extension. This function is called by CrossWalk
 * when loading the extension.
 */

int32_t XW_Initialize(XW_Extension extension, XW_GetInterface get_interface) {
    g_extension = extension;
    g_core = get_interface(XW_CORE_INTERFACE);
    g_core->SetExtensionName(extension, "MediaManager");

    g_core->SetJavaScriptAPI(extension, kSource_echo_api);
    g_core->RegisterInstanceCallbacks(
            extension, instance_created, instance_destroyed);
    g_core->RegisterShutdownCallback(extension, shutdown);

    g_messaging = get_interface(XW_MESSAGING_INTERFACE);
    g_messaging->Register(extension, handle_message);

    g_sync_messaging = get_interface(XW_INTERNAL_SYNC_MESSAGING_INTERFACE);
    g_sync_messaging->Register(extension, handle_sync_message);

    return XW_OK;
}
