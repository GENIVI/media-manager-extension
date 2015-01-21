#include <iostream>
#include <future>
#include <CommonAPI/CommonAPI.h>
#include "jansson.h"

#include <org/genivi/mediamanager/IndexerProxy.h>

#include "capi-client-indexer.h"
#include "rpc.h"

namespace MM = org::genivi::mediamanager;

bool CAPIClientIndexer::check_initialized () {
    if (!m_indexerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return false;
        }
    }
    return true;
}

bool CAPIClientIndexer::initialize () {
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
    const std::string commonApiAddress = "local:org.genivi.mediamanager.Indexer:org.genivi.mediamanager.Indexer";
    m_indexerProxy = factory->buildProxy<MM::IndexerProxy>(commonApiAddress);
    if (!m_indexerProxy) {
        std::cerr << "Error: Unable to build proxy!\n";
        return -1;
    }

    std::promise<CommonAPI::AvailabilityStatus> availabilityStatusPromise;
    m_indexerProxy->getProxyStatusEvent().subscribe([&](const CommonAPI::AvailabilityStatus& availabilityStatus) {
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

int CAPIClientIndexer::getDatabasePath (json_t *json_params, json_t **result, void *data) {
    CommonAPI::CallStatus callStatus;
    std::string databasePath;
    auto indexerError = MM::Indexer::IndexerError::NO_ERROR;

    if (!check_initialized())
        return -1;

    m_indexerProxy->getDatabasePath(callStatus, databasePath, indexerError);
    if (callStatus != CommonAPI::CallStatus::SUCCESS) {
        std::cerr << "Remote call failed!\n";
        return -1;
    }

    std::cout << "Database path: '"<< databasePath << "'" << std::endl;

    if (databasePath == "") {
        std::cerr << "Return value from backend invalid" << std::endl;
        return -1;
    }

    *result = json_string(databasePath.c_str());
    return 0;
}

int CAPIClientIndexer::startIndexing (json_t *json_params, json_t **result, void *data) {
    CommonAPI::CallStatus callStatus;
    auto indexerError = MM::Indexer::IndexerError::NO_ERROR;

    if (!check_initialized())
        return -1;

    m_indexerProxy->startIndexing(callStatus, indexerError);
    if (callStatus != CommonAPI::CallStatus::SUCCESS) {
        std::cerr << "Remote call failed!\n";
        return -1;
    }
    return 0;
}

int CAPIClientIndexer::stopIndexing (json_t *json_params, json_t **result, void *data) {
    CommonAPI::CallStatus callStatus;
    auto indexerError = MM::Indexer::IndexerError::NO_ERROR;

    if (!check_initialized())
        return -1;

    m_indexerProxy->stopIndexing(callStatus, indexerError);
    if (callStatus != CommonAPI::CallStatus::SUCCESS) {
        std::cerr << "Remote call failed!\n";
        return -1;
    }

    if (indexerError != MM::Indexer::IndexerError::NO_ERROR) {
        std::cerr << "Error returned from indexer!" << std::endl;
        return -1;
    }
    return 0;
}

int CAPIClientIndexer::getIndexerStatus (json_t *json_params, json_t **result, void *data) {
    return 0;
}

// C API

int capi_client_indexer_getDatabasePath (json_t *json_params, json_t **result, void *data) {
    CAPIClientIndexer ix;
    return ix.getDatabasePath(json_params, result, data);
}

int capi_client_indexer_startIndexing (json_t *json_params, json_t **result, void *data) {
    CAPIClientIndexer ix;
    return ix.startIndexing(json_params, result, data);
}

int capi_client_indexer_stopIndexing (json_t *json_params, json_t **result, void *data) {
    CAPIClientIndexer ix;
    return ix.stopIndexing(json_params, result, data);
}

int capi_client_indexer_getIndexerStatus (json_t *json_params, json_t **result, void *data) {
    CAPIClientIndexer ix;
    return ix.getIndexerStatus(json_params, result, data);
}
