#include <iostream>
#include <future>
#include <CommonAPI/CommonAPI.h>
#include "jansson.h"

#include <org/genivi/MediaManager/PlayerProxy.h>

#include "capi-client-player.h"
#include "rpc.h"
#include "inttypes.h"

CAPIClientPlayer* CAPIClientPlayer::m_instance = NULL;

CAPIClientPlayer* CAPIClientPlayer::instance() {
    if (!m_instance) {
        m_instance = new CAPIClientPlayer();
    }
    return m_instance;
}

void CAPIClientPlayer::registerEvents() {
    m_playerProxy->getPlaybackStatusAttribute().getChangedEvent().subscribe([&](const org::genivi::MediaManager::Player::PlaybackStatus& status) {
        std::cout << "Playback status changed!" << std::endl;
        if (status == org::genivi::MediaManager::Player::PlaybackStatus::PLAYING) {
            rpc_send_notification(xw_instance, "PlaybackStatus", "\"PLAYING\"");
        } else {
            rpc_send_notification(xw_instance, "PlaybackStatus", "\"PAUSED\"");
        }
    });

    m_playerProxy->getCurrentTrackAttribute().getChangedEvent().subscribe([&](const uint64_t& trackIdx) {
        std::cout << "Current track changed: " << trackIdx << std::endl;
        char trackIdxStr[10];
        sprintf(trackIdxStr, "%" PRIu64, trackIdx);
        rpc_send_notification(xw_instance, "CurrentTrack", trackIdxStr);
    });

    m_playerProxy->getMuteAttribute().getChangedEvent().subscribe([&](const org::genivi::MediaManager::Player::MuteStatus& status) {
        if (status == org::genivi::MediaManager::Player::MuteStatus::MUTED)
            rpc_send_notification(xw_instance, "Mute", "true");
        else
            rpc_send_notification(xw_instance, "Mute", "false");
    });

    m_playerProxy->getShuffleAttribute().getChangedEvent().subscribe([&](const org::genivi::MediaManager::Player::ShuffleStatus& status) {
        if (status == org::genivi::MediaManager::Player::ShuffleStatus::SHUFFLE)
            rpc_send_notification(xw_instance, "Shuffle", "true");
        else
            rpc_send_notification(xw_instance, "Shuffle", "false");
    });

    m_playerProxy->getRepeatAttribute().getChangedEvent().subscribe([&](const org::genivi::MediaManager::Player::RepeatStatus& status) {
        if (status == org::genivi::MediaManager::Player::RepeatStatus::REPEAT)
            rpc_send_notification(xw_instance, "Repeat", "true");
        else
            rpc_send_notification(xw_instance, "Repeat", "false");
    });

    m_playerProxy->getRateAttribute().getChangedEvent().subscribe([&](const org::genivi::MediaManager::Player::RateStatus& status) {
        int rate = 1;
        char rateStr[5];

        switch (status) {
            case org::genivi::MediaManager::Player::RateStatus::RATE_1:
                rate = 1;
                break;
            case org::genivi::MediaManager::Player::RateStatus::RATE_2:
                rate = 2;
                break;
            case org::genivi::MediaManager::Player::RateStatus::RATE_4:
                rate = 4;
                break;
            case org::genivi::MediaManager::Player::RateStatus::RATE_8:
                rate = 8;
                break;
            case org::genivi::MediaManager::Player::RateStatus::RATE_16:
                rate = 16;
                break;
            default:
                std::cout << "Unknown rate" << std::endl;
                return;
        }

        sprintf(rateStr, "%d", rate);

        rpc_send_notification(xw_instance, "Rate", "rateStr");
    });

    m_playerProxy->getVolumeAttribute().getChangedEvent().subscribe([&](const double& status) {
        char volumeStr[10];
        sprintf(volumeStr, "%f", status);
        rpc_send_notification(xw_instance, "Volume", volumeStr);
    });

    m_playerProxy->getCanGoNextAttribute().getChangedEvent().subscribe([&](const bool& status) {
        if (status)
            rpc_send_notification(xw_instance, "CanGoNext", "true");
        else
            rpc_send_notification(xw_instance, "CanGoNext", "false");
    });

    m_playerProxy->getCanGoPreviousAttribute().getChangedEvent().subscribe([&](const bool& status) {
        if (status)
            rpc_send_notification(xw_instance, "CanGoPrevious", "true");
        else
            rpc_send_notification(xw_instance, "CanGoPrevious", "false");
    });

    m_playerProxy->getCanPauseAttribute().getChangedEvent().subscribe([&](const bool& status) {
        if (status)
            rpc_send_notification(xw_instance, "CanPause", "true");
        else
            rpc_send_notification(xw_instance, "CanPause", "false");
    });

    m_playerProxy->getCanPlayAttribute().getChangedEvent().subscribe([&](const bool& status) {
        if (status)
            rpc_send_notification(xw_instance, "CanPlay", "true");
        else
            rpc_send_notification(xw_instance, "CanPlay", "false");
    });

    m_playerProxy->getCanSeekAttribute().getChangedEvent().subscribe([&](const bool& status) {
        if (status)
            rpc_send_notification(xw_instance, "CanSeek", "true");
        else
            rpc_send_notification(xw_instance, "CanSeek", "false");
    });
}

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

    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Player::PlaybackStatus playbackStatus;

    m_playerProxy->getPlaybackStatusAttribute().getValue(callStatus, playbackStatus);
    if (callStatus != CommonAPI::CallStatus::SUCCESS) {
        std::cout << "Failed to get playback status" << std::endl;
        return -1;
    }

    registerEvents();
    return true;
}

int CAPIClientPlayer::openUri (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
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
    std::cout << "In method " << __FUNCTION__ << std::endl;
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
    std::cout << "In method " << __FUNCTION__ << std::endl;
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
    std::cout << "In method " << __FUNCTION__ << std::endl;
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

int CAPIClientPlayer::next (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->next (callStatus, error);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::openPlaylist (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

    json_t *p0 = json_array_get(json_params, 0);
    std::string playlist   = std::string(json_string_value (p0));

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->openPlaylist (playlist, callStatus, error);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::previous (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->previous (callStatus, error);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::seek (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

    json_t *p0 = json_array_get(json_params, 0);
    long int seek   = json_integer_value (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->seek (seek, callStatus, error);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::setPosition (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

    json_t *p0 = json_array_get(json_params, 0);
    long int pos   = json_integer_value (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->seek (pos, callStatus, error);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::setMuted (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Player::MuteStatus response;

    json_t *p0 = json_array_get(json_params, 0);
    bool muted   = json_is_true (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getMuteAttribute().setValue(
        muted ? org::genivi::MediaManager::Player::MuteStatus::MUTED :
                org::genivi::MediaManager::Player::MuteStatus::UNMUTED ,
        callStatus,
        response);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::getMuted (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Player::MuteStatus response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getMuteAttribute().getValue(callStatus, response);
    if (response == org::genivi::MediaManager::Player::MuteStatus::MUTED) {
        *result = json_true();
    } else {
        *result = json_false();
    }
    return 0;
}
int CAPIClientPlayer::setShuffled (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Player::ShuffleStatus response;

    json_t *p0 = json_array_get(json_params, 0);
    bool b   = json_is_true (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getShuffleAttribute().setValue(
        b ? org::genivi::MediaManager::Player::ShuffleStatus::SHUFFLE :
            org::genivi::MediaManager::Player::ShuffleStatus::UNSHUFFLE ,
        callStatus,
        response);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::getShuffled (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Player::ShuffleStatus response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getShuffleAttribute().getValue(callStatus, response);
    if (response == org::genivi::MediaManager::Player::ShuffleStatus::SHUFFLE) {
        *result = json_true();
    } else {
        *result = json_false();
    }
    return 0;
}
int CAPIClientPlayer::setRepeated (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Player::RepeatStatus response;

    json_t *p0 = json_array_get(json_params, 0);
    bool b   = json_is_true (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getRepeatAttribute().setValue(
        b ? org::genivi::MediaManager::Player::RepeatStatus::REPEAT :
            org::genivi::MediaManager::Player::RepeatStatus::NO_REPEAT ,
        callStatus,
        response);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::getRepeated (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Player::RepeatStatus response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getRepeatAttribute().getValue(callStatus, response);
    if (response == org::genivi::MediaManager::Player::RepeatStatus::REPEAT) {
        *result = json_true();
    } else {
        *result = json_false();
    }
    return 0;
}
int CAPIClientPlayer::setRate (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Player::RateStatus response;
    org::genivi::MediaManager::Player::RateStatus request;

    json_t *p0 = json_array_get(json_params, 0);
    int rate   = json_integer_value (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    switch (rate) {
        case 1:
            request = org::genivi::MediaManager::Player::RateStatus::RATE_1;
            break;
        case 2:
            request = org::genivi::MediaManager::Player::RateStatus::RATE_2;
            break;
        case 4:
            request = org::genivi::MediaManager::Player::RateStatus::RATE_4;
            break;
        case 8:
            request = org::genivi::MediaManager::Player::RateStatus::RATE_8;
            break;
        case 16:
            request = org::genivi::MediaManager::Player::RateStatus::RATE_16;
            break;
    }

    m_playerProxy->getRateAttribute().setValue(
        request,
        callStatus,
        response);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::getRate (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Player::RateStatus response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getRateAttribute().getValue(callStatus, response);
    switch (response) {
        case org::genivi::MediaManager::Player::RateStatus::RATE_1:
            *result = json_integer(1);
            break;
        case org::genivi::MediaManager::Player::RateStatus::RATE_2:
            *result = json_integer(2);
            break;
        case org::genivi::MediaManager::Player::RateStatus::RATE_4:
            *result = json_integer(4);
            break;
        case org::genivi::MediaManager::Player::RateStatus::RATE_8:
            *result = json_integer(8);
            break;
        case org::genivi::MediaManager::Player::RateStatus::RATE_16:
            *result = json_integer(16);
            break;
    }
    return 0;
}
int CAPIClientPlayer::getVolume (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    double response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getVolumeAttribute().getValue(callStatus, response);
    *result = json_real(response);
    return 0;
}
int CAPIClientPlayer::setVolume (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    double response;

    json_t *p0 = json_array_get(json_params, 0);
    double rate   = json_real_value (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getVolumeAttribute().setValue(
        rate,
        callStatus,
        response);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::getCanGoNext (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    bool response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getCanGoNextAttribute().getValue(callStatus, response);
    if (response)
        *result = json_true();
    else
        *result = json_false();
    return 0;
}
int CAPIClientPlayer::getCanGoPrevious (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    bool response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getCanGoPreviousAttribute().getValue(callStatus, response);
    if (response)
        *result = json_true();
    else
        *result = json_false();
    return 0;
}
int CAPIClientPlayer::getCanPause (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    bool response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getCanPauseAttribute().getValue(callStatus, response);
    if (response)
        *result = json_true();
    else
        *result = json_false();
    return 0;
}
int CAPIClientPlayer::getCanPlay (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    bool response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getCanPlayAttribute().getValue(callStatus, response);
    if (response)
        *result = json_true();
    else
        *result = json_false();
    return 0;
}
int CAPIClientPlayer::getCanSeek (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    bool response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getCanSeekAttribute().getValue(callStatus, response);
    if (response)
        *result = json_true();
    else
        *result = json_false();
    return 0;
}
int CAPIClientPlayer::getCurrentTrack (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

//    #if JSON_INTEGER_IS_LONG_LONG
//    long long unsigned response;
//    #else
    long unsigned response;
//    #endif

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getCurrentTrackAttribute().getValue(callStatus, response);
    *result = json_integer(response);
    return 0;
}
int CAPIClientPlayer::getPlaybackStatus (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::MediaManager::Player::PlaybackStatus response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getPlaybackStatusAttribute().getValue(callStatus, response);
    if (response == org::genivi::MediaManager::Player::PlaybackStatus::PLAYING)
        *result = json_string("PLAYING");
    else
        *result = json_string("PAUSED");
    return 0;
}
int CAPIClientPlayer::getPosition (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::MediaManager::Player::PlayerError error;
    CommonAPI::CallStatus callStatus;

//    #if JSON_INTEGER_IS_LONG_LONG
//    long long unsigned response;
//    #else
    long unsigned response;
//    #endif

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getPositionAttribute().getValue(callStatus, response);
    *result = json_integer(response);
    return 0;
}

void capi_client_player_set_xwalk_instance (XW_Instance instance) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    b->xw_instance = instance;
}

int capi_client_player_openUri (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->openUri(json_params, result, data);
}

int capi_client_player_pause (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->pause(json_params, result, data);
}

int capi_client_player_play (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->play(json_params, result, data);
}

int capi_client_player_playPause (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->playPause(json_params, result, data);
}
int capi_client_player_next (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->next(json_params, result, data);
}
int capi_client_player_openPlaylist (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->openPlaylist(json_params, result, data);
}
int capi_client_player_previous (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->previous(json_params, result, data);
}
int capi_client_player_seek (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->seek(json_params, result, data);
}
int capi_client_player_setPosition (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->setPosition(json_params, result, data);
}
int capi_client_player_setMuted (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->setMuted(json_params, result, data);
}
int capi_client_player_getMuted (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getMuted(json_params, result, data);
}
int capi_client_player_setShuffled (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->setShuffled(json_params, result, data);
}
int capi_client_player_getShuffled (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getShuffled(json_params, result, data);
}
int capi_client_player_setRepeated (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->setRepeated(json_params, result, data);
}
int capi_client_player_getRepeated (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getRepeated(json_params, result, data);
}
int capi_client_player_setRate (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->setRate(json_params, result, data);
}
int capi_client_player_getRate (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getRate(json_params, result, data);
}
int capi_client_player_getVolume (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getVolume(json_params, result, data);
}
int capi_client_player_setVolume (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->setVolume(json_params, result, data);
}
int capi_client_player_getCanGoNext (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getCanGoNext(json_params, result, data);
}
int capi_client_player_getCanGoPrevious (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getCanGoPrevious(json_params, result, data);
}
int capi_client_player_getCanPause (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getCanPause(json_params, result, data);
}
int capi_client_player_getCanPlay (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getCanPlay(json_params, result, data);
}
int capi_client_player_getCanSeek (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getCanSeek(json_params, result, data);
}
int capi_client_player_getCurrentTrack (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getCurrentTrack(json_params, result, data);
}
int capi_client_player_getPlaybackStatus (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getPlaybackStatus(json_params, result, data);
}
int capi_client_player_getPosition (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getPosition(json_params, result, data);
}
