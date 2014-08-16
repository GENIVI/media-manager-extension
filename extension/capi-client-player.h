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
#include "XW_Extension.h"

#ifdef __cplusplus
#include <org/genivi/MediaManager/PlayerProxy.h>

class CAPIClientPlayer {
public:

XW_Instance xw_instance;
static CAPIClientPlayer *instance();

int openUri(json_t *json_params, json_t **result, void *data);
int pause (json_t *json_params,  json_t **result, void *data);
int play (json_t *json_params,  json_t **result, void *data);
int playPause (json_t *json_params,  json_t **result, void *data);
int next (json_t *json_params,  json_t **result, void *data);
int openPlaylist (json_t *json_params,  json_t **result, void *data);
int previous (json_t *json_params,  json_t **result, void *data);
int seek (json_t *json_params,  json_t **result, void *data);
int setPosition (json_t *json_params,  json_t **result, void *data);
int setMuted (json_t *json_params,  json_t **result, void *data);
int getMuted (json_t *json_params,  json_t **result, void *data);
int setShuffled (json_t *json_params,  json_t **result, void *data);
int getShuffled (json_t *json_params,  json_t **result, void *data);
int setRepeated (json_t *json_params,  json_t **result, void *data);
int getRepeated (json_t *json_params,  json_t **result, void *data);
int setRate (json_t *json_params,  json_t **result, void *data);
int getRate (json_t *json_params,  json_t **result, void *data);
int getVolume (json_t *json_params,  json_t **result, void *data);
int setVolume (json_t *json_params,  json_t **result, void *data);
int getCanGoNext (json_t *json_params,  json_t **result, void *data);
int getCanGoPrevious (json_t *json_params,  json_t **result, void *data);
int getCanPause (json_t *json_params,  json_t **result, void *data);
int getCanPlay (json_t *json_params,  json_t **result, void *data);
int getCanSeek (json_t *json_params,  json_t **result, void *data);
int getCurrentTrack (json_t *json_params,  json_t **result, void *data);
int getPlaybackStatus (json_t *json_params,  json_t **result, void *data);
int getPosition (json_t *json_params,  json_t **result, void *data);

private:
bool initialize();
std::shared_ptr<org::genivi::MediaManager::PlayerProxy<> > m_playerProxy;
static CAPIClientPlayer *m_instance;
};

extern "C" {
#endif
    void capi_client_player_set_xwalk_instance(XW_Extension instance);

    int capi_client_player_openUri (json_t *json_params, json_t **result, void *data);
    int capi_client_player_pause (json_t *json_params, json_t **result, void *data);
    int capi_client_player_play (json_t *json_params, json_t **result, void *data);
    int capi_client_player_playPause (json_t *json_params, json_t **result, void *data);
    int capi_client_player_next (json_t *json_params, json_t **result, void *data);
    int capi_client_player_openPlaylist (json_t *json_params, json_t **result, void *data);
    int capi_client_player_previous (json_t *json_params, json_t **result, void *data);
    int capi_client_player_seek (json_t *json_params, json_t **result, void *data);
    int capi_client_player_setPosition (json_t *json_params, json_t **result, void *data);
    int capi_client_player_setMuted (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getMuted (json_t *json_params, json_t **result, void *data);
    int capi_client_player_setShuffled (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getShuffled (json_t *json_params, json_t **result, void *data);
    int capi_client_player_setRepeated (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getRepeated (json_t *json_params, json_t **result, void *data);
    int capi_client_player_setRate (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getRate (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getVolume (json_t *json_params, json_t **result, void *data);
    int capi_client_player_setVolume (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getCanGoNext (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getCanGoPrevious (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getCanPause (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getCanPlay (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getCanSeek (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getCurrentTrack (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getPlaybackStatus (json_t *json_params, json_t **result, void *data);
    int capi_client_player_getPosition (json_t *json_params, json_t **result, void *data);
#ifdef __cplusplus
}
#endif

#endif /* CAPI_CLIENT_PLAYER_H */
