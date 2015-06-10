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

#include "common.h"

void Common::BrowserMapListToJSON (const org::genivi::mediamanager::MediaTypes::ResultMapList& from,
                                   json_t **to) {
    json_t *newList = json_array();

    for (auto elem : from) {
        json_t* obj = json_object();
        for (auto kvp : elem) {
            auto key = kvp.first;
            std::cout << key << std::endl;
            auto value = kvp.second;
            if (value.isType<std::string>()) {
                std::string valueStr = value.get<std::string>();
                json_object_set_new(obj, key.c_str(), json_string(valueStr.c_str()));
            } else if (value.isType<int32_t>()) {
                int32_t valueInt = value.get<int32_t>();
                json_object_set_new(obj, key.c_str(), json_integer(valueInt));
            } else if (value.isType<int64_t>()) {
                int64_t valueInt = value.get<int64_t>();
                json_object_set_new(obj, key.c_str(), json_integer(valueInt));
            } else if (value.isType<uint32_t>()) {
                uint32_t valueInt = value.get<uint32_t>();
                json_object_set_new(obj, key.c_str(), json_integer(valueInt));
            } else if (value.isType<uint64_t>()) {
                uint64_t valueInt = value.get<uint64_t>();
                json_object_set_new(obj, key.c_str(), json_integer(valueInt));
            } else if (value.isType<bool>()) {
                bool valueBool = value.get<bool>();
                if (valueBool)
                    json_object_set(obj, key.c_str(), json_true());
                else
                    json_object_set(obj, key.c_str(), json_false());
            } else if (value.isType<double>()) {
                double valueDouble = value.get<double>();
                json_object_set_new(obj, key.c_str(), json_real(valueDouble));
            } else if (value.isType<std::vector<std::string> >()) {
                std::vector<std::string> valueStrVec = value.get<std::vector<std::string> >();
                json_t* jsonStrVec = json_array();
                for (auto v : valueStrVec) {
                    json_array_append(jsonStrVec, json_string(v.c_str()));
                }
                json_object_set_new(obj, key.c_str(), jsonStrVec);
            }
            else {
                std::cout << "Warning: Unhandled CommonAPI type: " << std::to_string(value.getValueType()) << std::endl;
            }
        }
        json_array_append_new (newList, obj);
    }
    *to = json_string(json_dumps(newList, 0));
}
