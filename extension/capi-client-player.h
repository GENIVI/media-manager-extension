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

#ifndef CAPI_CLIENT_PLAYER_H
#define CAPI_CLIENT_PLAYER_H
#include "jansson.h"

#ifdef __cplusplus
#include <org/genivi/MediaManager/PlayerProxy.h>

class CAPIClientPlayer {
public:
int openUri(json_t *json_params, json_t **result, void *data);
int pause (json_t *json_params,  json_t **result, void *data);
int play (json_t *json_params,  json_t **result, void *data);
int playPause (json_t *json_params,  json_t **result, void *data);

private:
bool initialize();
std::shared_ptr<org::genivi::MediaManager::PlayerProxy<> > m_playerProxy;
};

extern "C" {
#endif
    int capi_client_player_openUri (json_t *json_params, json_t **result, void *data);
    int capi_client_player_pause (json_t *json_params, json_t **result, void *data);
    int capi_client_player_play (json_t *json_params, json_t **result, void *data);
    int capi_client_player_playPause (json_t *json_params, json_t **result, void *data);
#ifdef __cplusplus
}
#endif

#endif /* CAPI_CLIENT_PLAYER_H */
