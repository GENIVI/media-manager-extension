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

#ifndef CAPI_CLIENT_BROWSER_H
#define CAPI_CLIENT_BROWSER_H
#include "jansson.h"

#ifdef __cplusplus
#include <org/genivi/MediaManager/BrowserProxy.h>

class CAPIClientBrowser {
public:
int listContainers(json_t *json_params, json_t **result, void *data);
int listItems(json_t *json_params, json_t **result, void *data);
int createReference(json_t *json_params, json_t **result, void *data);
int createContainer(json_t *json_params, json_t **result, void *data);

private:
bool initialize();
std::shared_ptr<org::genivi::MediaManager::BrowserProxy<> > m_browserProxy;
};

extern "C" {
#endif
    int capi_client_browser_listContainers (json_t *json_params, json_t **result, void *data);
    int capi_client_browser_listItems (json_t *json_params, json_t **result, void *data);
    int capi_client_browser_createReference (json_t *json_params, json_t **result, void *data);
    int capi_client_browser_createContainer (json_t *json_params, json_t **result, void *data);
#ifdef __cplusplus
}
#endif

#endif /* CAPI_CLIENT_H */
