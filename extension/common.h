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

#include "jansson.h"

#include <org/genivi/mediamanager/BrowserProxy.h>

class Common {
public:
    /** Convert a BrowserMapList to JSON form **/
    static void BrowserMapListToJSON (const org::genivi::mediamanager::MediaTypes::ResultMapList& from,
                               json_t **to);
};
