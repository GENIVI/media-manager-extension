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

#ifndef CAPI_CLIENT_H
#define CAPI_CLIENT_H
#include "jansson.h"

#ifdef __cplusplus
#include <org/genivi/mediamanager/IndexerProxy.h>

/**
 * CommonAPI proxy for the Player interface. The functions in this class are
 * direct connections to the CommonAPI functions of the Media Manager core
 * component. Please see the documentation of this conponent.
 */
class CAPIClientIndexer {
public:
int getDatabasePath(json_t *json_params, json_t **result, void *data);
int startIndexing(json_t *json_params, json_t **result, void *data);
int stopIndexing(json_t *json_params, json_t **result, void *data);
int getIndexerStatus(json_t *json_params, json_t **result, void *data);

private:
bool check_initialized();
bool initialize();
std::shared_ptr<org::genivi::mediamanager::IndexerProxy<> > m_indexerProxy;
};

extern "C" {
#endif
    int capi_client_indexer_getDatabasePath (json_t *json_params, json_t **result, void *data);
    int capi_client_indexer_startIndexing (json_t *json_params, json_t **result, void *data);
    int capi_client_indexer_stopIndexing (json_t *json_params, json_t **result, void *data);
    int capi_client_indexer_getIndexerStatus (json_t *json_params, json_t **result, void *data);
#ifdef __cplusplus
}
#endif

#endif /* CAPI_CLIENT_H */
