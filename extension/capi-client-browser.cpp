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
        return -1;
    }

    auto factory = runtime->createFactory();
    if (!factory) {
        std::cerr << "Error: Unable to create factory!\n";
        return -1;
    }
    const std::string commonApiAddress = "local:org.genivi.MediaManager.Browser:org.genivi.MediaManager.Browser";
    m_browserProxy = factory->buildProxy<org::genivi::MediaManager::BrowserProxy>(commonApiAddress);
    if (!m_browserProxy) {
        std::cerr << "Error: Unable to build browser proxy!\n";
        return -1;
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
        return -1;
    }

    return true;
}

int CAPIClientBrowser::listContainers (json_t *json_params, json_t **result, void *data) {
    if (!m_browserProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    *result = json_string("Not implemented");
    return 0;
}

int capi_client_browser_listContainers (json_t *json_params, json_t **result, void *data) {
    CAPIClientBrowser b;
    return b.listContainers(json_params, result, data);
}
