#include <iostream>
#include <future>
#include <CommonAPI/CommonAPI.h>
#include "jansson.h"

#include <org/genivi/MediaManager/PlayerProxy.h>

#include "capi-client-player.h"
#include "rpc.h"

bool CAPIClientPlayer::initialize () {
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
    const std::string commonApiAddress = "local:org.genivi.MediaManager.Player:org.genivi.MediaManager.Player";
    m_playerProxy = factory->buildProxy<org::genivi::MediaManager::PlayerProxy>(commonApiAddress);
    if (!m_playerProxy) {
        std::cerr << "Error: Unable to build player proxy!\n";
        return -1;
    }

    std::promise<CommonAPI::AvailabilityStatus> availabilityStatusPromise;
    m_playerProxy->getProxyStatusEvent().subscribe([&](const CommonAPI::AvailabilityStatus& availabilityStatus) {
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

int CAPIClientPlayer::openUri (json_t *json_params, json_t **result, void *data) {
    const char *uri;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

    json_t *p0 = json_array_get(json_params, 0);
    uri   = json_string_value (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->openUri (uri, callStatus, error);
    *result = json_string("");
    return 0;
}

int CAPIClientPlayer::pause (json_t *json_params,  json_t **result, void *data) {
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->pause (callStatus, error);
    *result = json_string("");
    return 0;
}

int CAPIClientPlayer::play (json_t *json_params,  json_t **result, void *data) {
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->play (callStatus, error);
    *result = json_string("");
    return 0;
}

int CAPIClientPlayer::playPause (json_t *json_params,  json_t **result, void *data) {
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->playPause (callStatus, error);
    *result = json_string("");
    return 0;
}

int capi_client_player_openUri (json_t *json_params, json_t **result, void *data) {
    CAPIClientPlayer b;
    return b.openUri(json_params, result, data);
}

int capi_client_player_pause (json_t *json_params, json_t **result, void *data) {
    CAPIClientPlayer b;
    return b.pause(json_params, result, data);
}

int capi_client_player_play (json_t *json_params, json_t **result, void *data) {
    CAPIClientPlayer b;
    return b.play(json_params, result, data);
}

int capi_client_player_playPause (json_t *json_params, json_t **result, void *data) {
    CAPIClientPlayer b;
    return b.playPause(json_params, result, data);
}
