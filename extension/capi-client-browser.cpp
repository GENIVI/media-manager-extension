#include <iostream>
#include <future>
#include <CommonAPI/CommonAPI.h>
#include "jansson.h"

#include <org/genivi/MediaManager/BrowserProxy.h>

#include "capi-client-browser.h"
#include "rpc.h"

bool CAPIClientBrowser::initialize () {
    CommonAPI::Runtime::LoadState loadState;
    auto runtime = CommonAPI::Runtime::load(loadState);
    if (loadState != CommonAPI::Runtime::LoadState::SUCCESS) {
        std::cerr << "Error: Unable to load runtime!\n";
        return false;
    }

    auto factory = runtime->createFactory();
    if (!factory) {
        std::cerr << "Error: Unable to create factory!\n";
        return false;
    }
    const std::string commonApiAddress = "local:org.genivi.MediaManager.Browser:org.genivi.MediaManager.Browser";
    m_browserProxy = factory->buildProxy<org::genivi::MediaManager::BrowserProxy>(commonApiAddress);
    if (!m_browserProxy) {
        std::cerr << "Error: Unable to build browser proxy!\n";
        return false;
    }

    std::promise<CommonAPI::AvailabilityStatus> availabilityStatusPromise;
    m_browserProxy->getProxyStatusEvent().subscribe([&](const CommonAPI::AvailabilityStatus& availabilityStatus) {
            availabilityStatusPromise.set_value(availabilityStatus);
            return CommonAPI::SubscriptionStatus::CANCEL;
    });

    auto availabilityStatusFuture = availabilityStatusPromise.get_future();
    availabilityStatusFuture.wait();

    if (availabilityStatusFuture.get() != CommonAPI::AvailabilityStatus::AVAILABLE) {
        std::cerr << "Proxy not available!\n";
        return false;
    }

    return true;
}

int CAPIClientBrowser::discoverMediaManagers (json_t *json_params, json_t **result, void *data) {
    CommonAPI::CallStatus status;
    org::genivi::MediaManager::Browser::BrowserError error;
    *result = json_array();
    std::vector<std::string> managers;

    if (!m_browserProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for browser" << std::endl;
            return -1;
        }
    }

    m_browserProxy->discoverMediaManagers (status,
                                           managers,
                                           error);
    for (int i = 0; i < managers.size(); i++) {
        json_array_append(*result, json_string(managers[i].c_str()));
    }
    return 0;
}
int CAPIClientBrowser::listContainers (json_t *json_params, json_t **result, void *data, bool ex) {
    std::vector<std::string> filter;
    CommonAPI::CallStatus status;
    std::string json;
    org::genivi::MediaManager::Browser::BrowserError error;
    int offset, count;
    const char *path;

    json_t *p0 = json_array_get(json_params, 0);
    json_t *p1 = json_array_get(json_params, 1);
    json_t *p2 = json_array_get(json_params, 2);
    json_t *p3 = json_array_get(json_params, 3);
    {
        size_t index;
        json_t *value;
        size_t size = json_array_size(p3);

        for (int i = 0; i < size; i++) {
            value = json_array_get (p3, i);
            filter.push_back(json_string_value (value));
        }
    }

    path   = json_string_value (p0);
    offset = json_integer_value (p1);
    count  = json_integer_value (p2);

    if (!m_browserProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for browser" << std::endl;
            return -1;
        }
    }

    if (ex) {
        std::string sortKeyStr = json_string_value(json_array_get(json_params, 4));
        org::genivi::MediaManager::Browser::SortKey sortKey;
        sortKey = sortKeyStringToSortKey (sortKeyStr);
        m_browserProxy->listContainersEx (path,
                                          offset,
                                          count,
                                          filter,
                                          sortKey,
                                          status,
                                          json,
                                          error);
    } else
        m_browserProxy->listContainers (path,
                                        offset,
                                        count,
                                        filter,
                                        status,
                                        json,
                                        error);

    *result = json_string(json.c_str());
    return 0;
}

int CAPIClientBrowser::listItems (json_t *json_params, json_t **result, void *data, bool ex) {
    std::vector<std::string> filter;
    CommonAPI::CallStatus callStatus;
    std::string json;
    org::genivi::MediaManager::Browser::BrowserError error;
    int offset, count;
    const char *path, *sortKeyStr;

    json_t *p0 = json_array_get(json_params, 0);
    json_t *p1 = json_array_get(json_params, 1);
    json_t *p2 = json_array_get(json_params, 2);
    json_t *p3 = json_array_get(json_params, 3);
    {
        size_t index;
        json_t *value;
        size_t size = json_array_size(p3);

        for (int i = 0; i < size; i++) {
            value = json_array_get (p3, i);
            filter.push_back(json_string_value (value));
        }
    }

    path   = json_string_value (p0);
    offset = json_integer_value (p1);
    count  = json_integer_value (p2);

    if (!m_browserProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for browser" << std::endl;
            return -1;
        }
    }

    if (ex) {
        std::string sortKeyStr = json_string_value(json_array_get(json_params, 4));
        org::genivi::MediaManager::Browser::SortKey sortKey;
        sortKey = sortKeyStringToSortKey (sortKeyStr);
        m_browserProxy->listItemsEx (path,
                                    offset,
                                    count,
                                    filter,
                                    sortKey,
                                    callStatus,
                                    json,
                                    error);
    } else {
        m_browserProxy->listItems (path,
                                   offset,
                                   count,
                                   filter,
                                   callStatus,
                                   json,
                                   error);
    }

    if (callStatus != CommonAPI::CallStatus::SUCCESS) {
        std::cerr << "Remote call failed!\n";
        return -1;
    }

    *result = json_string(json.c_str());
    return 0;
}

int CAPIClientBrowser::createReference (json_t *json_params, json_t **result, void *data) {
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Browser::BrowserError error;
    const char *path, *object;
    std::string newPath;

    json_t *p0 = json_array_get(json_params, 0);
    json_t *p1 = json_array_get(json_params, 1);
    path   = json_string_value (p0);
    object   = json_string_value (p1);

    if (!m_browserProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for browser" << std::endl;
            return -1;
        }
    }

    m_browserProxy->createReference (path,
                                    object,
                                    callStatus,
                                    newPath,
                                    error);

    if (callStatus != CommonAPI::CallStatus::SUCCESS) {
        std::cerr << "Remote call failed!\n";
        return -1;
    }

    *result = json_string("");
    return 0;
}

int CAPIClientBrowser::createContainer (json_t *json_params, json_t **result, void *data) {
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Browser::BrowserError error;
    const char *path, *container;
    std::string newPath;
    std::vector<std::string> childTypes;

    json_t *p0 = json_array_get(json_params, 0);
    json_t *p1 = json_array_get(json_params, 1);
    path      = json_string_value (p0);
    container = json_string_value (p1);

    childTypes.push_back("container");

    if (!m_browserProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for browser" << std::endl;
            return -1;
        }
    }

    m_browserProxy->createContainer (path,
                                    container,
                                    childTypes,
                                    callStatus,
                                    newPath,
                                    error);

    if (callStatus != CommonAPI::CallStatus::SUCCESS) {
        std::cerr << "Remote call failed!\n";
        return -1;
    }

    *result = json_string(newPath.c_str());
    return 0;
}

int CAPIClientBrowser::searchObjects(json_t *json_params, json_t **result, void *data, bool ex) {
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Browser::BrowserError error;
    const char *query, *container;
    int offset, count;
    std::vector<std::string> filter;
    std::string resultStr;

    json_t *p0 = json_array_get(json_params, 0);
    json_t *p1 = json_array_get(json_params, 1);
    json_t *p2 = json_array_get(json_params, 2);
    json_t *p3 = json_array_get(json_params, 3);
    container = json_string_value (p0);
    offset = json_integer_value (p1);
    count = json_integer_value (p2);

    {
        size_t index;
        json_t *value;
        size_t size = json_array_size(p3);

        for (int i = 0; i < size; i++) {
            value = json_array_get (p3, i);
            filter.push_back(json_string_value (value));
        }
    }

    query = json_string_value(json_array_get(json_params, 4));

    if (!m_browserProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for browser" << std::endl;
            return -1;
        }
    }

    if (ex) {
        std::string sortKeyStr = json_string_value(json_array_get(json_params, 4));
        org::genivi::MediaManager::Browser::SortKey sortKey;
        sortKey = sortKeyStringToSortKey (sortKeyStr);
        m_browserProxy->searchObjectsEx (container,
                                         query,
                                         offset,
                                         count,
                                         filter,
                                         sortKey,
                                         callStatus,
                                         resultStr,
                                         error);
    } else {
        m_browserProxy->searchObjects (container,
                                       query,
                                       offset,
                                       count,
                                       filter,
                                       callStatus,
                                       resultStr,
                                       error);
    }

    if (callStatus != CommonAPI::CallStatus::SUCCESS) {
        std::cerr << "Remote call failed!\n";
        return -1;
    }

    *result = json_string(resultStr.c_str());
    return 0;
}

int CAPIClientBrowser::listChildren(json_t *json_params, json_t **result, void *data, bool ex){return 0;}
int CAPIClientBrowser::listIndexes(json_t *json_params, json_t **result, void *data){return 0;}

std::string CAPIClientBrowser::sortKeyToString (org::genivi::MediaManager::Browser::SortKey sk) {
    std::string keyStr;
    if (sk.order == org::genivi::MediaManager::Browser::SortOrder::ASCENDING)
        keyStr += "+";
    else
        keyStr += "-";

    keyStr += sk.keyName;

    return keyStr;
}

org::genivi::MediaManager::Browser::SortKey CAPIClientBrowser::sortKeyStringToSortKey (std::string strKey) {
    org::genivi::MediaManager::Browser::SortKey key;
    if (strKey[0] == '+')
        key.order = org::genivi::MediaManager::Browser::SortOrder::ASCENDING;
    else if (strKey[0] == '-')
        key.order = org::genivi::MediaManager::Browser::SortOrder::DESCENDING;
    else
        std::cout << "Unknown sort order '" << strKey[0] << "'" << std::endl;

    key.keyName = strKey.substr(1);
}

int capi_client_browser_discoverMediaManagers (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.discoverMediaManagers(json_params, result, data);
}

int capi_client_browser_listContainers (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.listContainers(json_params, result, data);
}

int capi_client_browser_listItems (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.listItems(json_params, result, data);
}

int capi_client_browser_createReference (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.createReference(json_params, result, data);
}

int capi_client_browser_createContainer (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.createContainer(json_params, result, data);
}
int capi_client_browser_listContainersEx (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.listContainers(json_params, result, data, true);
}
int capi_client_browser_listItemsEx (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.listItems(json_params, result, data, true);
}
int capi_client_browser_searchObjects (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.searchObjects(json_params, result, data);
}
int capi_client_browser_searchObjectsEx (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.searchObjects(json_params, result, data, true);
}
int capi_client_browser_listIndexes (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.listIndexes(json_params, result, data);
}
int capi_client_browser_listChildren (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.listChildren(json_params, result, data);
}
int capi_client_browser_listChildrenEx (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.listChildren(json_params, result, data, true);
}
