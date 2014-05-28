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
#ifndef RPC_H
#define RPC_H

#include "jsonrpc.h"
/*! Various error values for RPC commands. These are typically included in
 * JSON-RPC error responses */
typedef enum {
    /*! Used to signal to client that backend (LMS, for example) is unreachable */
    RPC_ERROR_BACKEND_UNREACHABLE = -32001,

    /*! Used to indicate that backend did reply, but the reply was invalid */
    RPC_ERROR_BACKEND_REPLY_INVALID = -32002,

    /*! No error encountered */
    RPC_ERROR_OK = 0
} rpc_error_t;

/*!
 * Handle an incoming RPC message. Message needs to be a valid JSON-RPC 2.0
 * message.
 *
 * \param message: Valid JSON-RPC 2.0 char*
 * \returns: JSON-RPC 2.0 reply if \p message is valid
 * \returns: NULL if \p message is invalid
 */
char *rpc_handle_message (const char *message);

#endif /* RPC_H */
