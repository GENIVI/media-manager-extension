#include <iostream>
#include <future>
#include <CommonAPI/CommonAPI.h>
#include "jansson.h"

#include <org/genivi/mediamanager/PlayerProxy.h>

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
    m_playerProxy->getPlaybackStatusAttribute().getChangedEvent().subscribe([&](const org::genivi::mediamanager::PlayerTypes::PlaybackStatus& status) {
        std::cout << "Playback status changed!" << std::endl;
        if (status == org::genivi::mediamanager::PlayerTypes::PlaybackStatus::PLAYING) {
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

    m_playerProxy->getMuteAttribute().getChangedEvent().subscribe([&](const org::genivi::mediamanager::PlayerTypes::MuteStatus& status) {
        if (status == org::genivi::mediamanager::PlayerTypes::MuteStatus::MUTED)
            rpc_send_notification(xw_instance, "Mute", "true");
        else
            rpc_send_notification(xw_instance, "Mute", "false");
    });

    m_playerProxy->getShuffleAttribute().getChangedEvent().subscribe([&](const org::genivi::mediamanager::PlayerTypes::ShuffleStatus& status) {
        if (status == org::genivi::mediamanager::PlayerTypes::ShuffleStatus::SHUFFLE)
            rpc_send_notification(xw_instance, "Shuffle", "true");
        else
            rpc_send_notification(xw_instance, "Shuffle", "false");
    });

    m_playerProxy->getRepeatAttribute().getChangedEvent().subscribe([&](const org::genivi::mediamanager::PlayerTypes::RepeatStatus& status) {
        if (status == org::genivi::mediamanager::PlayerTypes::RepeatStatus::REPEAT) {
            rpc_send_notification(xw_instance, "Repeat", "\"REPEAT\"");
        } else if (status == org::genivi::mediamanager::PlayerTypes::RepeatStatus::REPEAT_SINGLE) {
            rpc_send_notification(xw_instance, "Repeat", "\"REPEAT_SINGLE\"");
        } else if (status == org::genivi::mediamanager::PlayerTypes::RepeatStatus::NO_REPEAT) {
            rpc_send_notification(xw_instance, "Repeat", "\"NO_REPEAT\"");
        } else {
            std::cout << "Unknown repeat value, should be REPEAT, NO_REPEAT or REPEAT_SINGLE" << std::endl;
        }
    });

    m_playerProxy->getRateAttribute().getChangedEvent().subscribe([&](const double& rate) {
        char rateStr[5];
        sprintf(rateStr, "%d", rate);

        rpc_send_notification(xw_instance, "Rate", rateStr);
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
    const std::string commonApiAddress = "local:org.genivi.mediamanager.Player:org.genivi.mediamanager.Player";
    m_playerProxy = factory->buildProxy<org::genivi::mediamanager::PlayerProxy>(commonApiAddress);
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
    org::genivi::mediamanager::PlayerTypes::PlaybackStatus playbackStatus;

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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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

int CAPIClientPlayer::stop (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->pause (callStatus, error);
    m_playerProxy->setPosition (0, callStatus, error);
    *result = json_string("");
    return 0;
}

int CAPIClientPlayer::playPause (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;

    json_t *p0 = json_array_get(json_params, 0);
    long int pos   = json_integer_value (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->setPosition (pos, callStatus, error);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::setMuted (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::mediamanager::PlayerTypes::MuteStatus response;

    json_t *p0 = json_array_get(json_params, 0);
    bool muted   = json_is_true (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getMuteAttribute().setValue(
        muted ? org::genivi::mediamanager::PlayerTypes::MuteStatus::MUTED :
                org::genivi::mediamanager::PlayerTypes::MuteStatus::UNMUTED ,
        callStatus,
        response);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::getMuted (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::mediamanager::PlayerTypes::MuteStatus response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getMuteAttribute().getValue(callStatus, response);
    if (response == org::genivi::mediamanager::PlayerTypes::MuteStatus::MUTED) {
        *result = json_true();
    } else {
        *result = json_false();
    }
    return 0;
}
int CAPIClientPlayer::setShuffled (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::mediamanager::PlayerTypes::ShuffleStatus response;

    json_t *p0 = json_array_get(json_params, 0);
    bool b   = json_is_true (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getShuffleAttribute().setValue(
        b ? org::genivi::mediamanager::PlayerTypes::ShuffleStatus::SHUFFLE :
            org::genivi::mediamanager::PlayerTypes::ShuffleStatus::UNSHUFFLE ,
        callStatus,
        response);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::getShuffled (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::mediamanager::PlayerTypes::ShuffleStatus response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getShuffleAttribute().getValue(callStatus, response);
    if (response == org::genivi::mediamanager::PlayerTypes::ShuffleStatus::SHUFFLE) {
        *result = json_true();
    } else {
        *result = json_false();
    }
    return 0;
}
int CAPIClientPlayer::setRepeated (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::mediamanager::PlayerTypes::RepeatStatus response;

    json_t *p0 = json_array_get(json_params, 0);
    std::string status   = std::string(json_string_value (p0));

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    if (status == "REPEAT") {
        m_playerProxy->getRepeatAttribute().setValue(
            org::genivi::mediamanager::PlayerTypes::RepeatStatus::REPEAT,
            callStatus,
            response);
    } else if (status == "REPEAT_SINGLE") {
        m_playerProxy->getRepeatAttribute().setValue(
            org::genivi::mediamanager::PlayerTypes::RepeatStatus::REPEAT_SINGLE,
            callStatus,
            response);
    } else if (status == "NO_REPEAT") {
        m_playerProxy->getRepeatAttribute().setValue(
            org::genivi::mediamanager::PlayerTypes::RepeatStatus::NO_REPEAT,
            callStatus,
            response);
    } else {
        std::cout << "Unknown repeat value: " << status << ", should be REPEAT, NO_REPEAT or REPEAT_SINGLE" << std::endl;
    }
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::getRepeated (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::mediamanager::PlayerTypes::RepeatStatus response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getRepeatAttribute().getValue(callStatus, response);
    if (response == org::genivi::mediamanager::PlayerTypes::RepeatStatus::REPEAT) {
        *result = json_string("REPEAT");
    } else if (response == org::genivi::mediamanager::PlayerTypes::RepeatStatus::REPEAT_SINGLE) {
        *result = json_string("REPEAT_SINGLE");
    } else if (response == org::genivi::mediamanager::PlayerTypes::RepeatStatus::NO_REPEAT) {
        *result = json_string("NO_REPEAT");
    } else {
        std::cout << "Unknown repeat value, should be REPEAT, NO_REPEAT or REPEAT_SINGLE" << std::endl;
    }
    return 0;
}
int CAPIClientPlayer::setRate (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    double response;

    json_t *p0 = json_array_get(json_params, 0);
    int rate   = json_integer_value (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getRateAttribute().setValue(
        rate,
        callStatus,
        response);
    *result = json_string("");
    return 0;
}
int CAPIClientPlayer::getRate (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    double response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getRateAttribute().getValue(callStatus, response);
    *result = json_integer(response);
    return 0;
}
int CAPIClientPlayer::getVolume (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;

    #if __x86_64__
    long unsigned response;
    #else
    long long unsigned response;
    #endif

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
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::mediamanager::PlayerTypes::PlaybackStatus response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getPlaybackStatusAttribute().getValue(callStatus, response);
    if (response == org::genivi::mediamanager::PlayerTypes::PlaybackStatus::PLAYING)
        *result = json_string("PLAYING");
    else
        *result = json_string("PAUSED");
    return 0;
}
int CAPIClientPlayer::getPosition (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;

    #if __x86_64__
    long unsigned response;
    #else
    long long unsigned response;
    #endif

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

int CAPIClientPlayer::getDuration (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;

    #if __x86_64__
    long unsigned response;
    #else
    long long unsigned response;
    #endif

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->getDurationAttribute().getValue(callStatus, response);
    *result = json_integer(response);
    return 0;
}

int CAPIClientPlayer::getCurrentPlayQueue (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    org::genivi::mediamanager::MediaTypes::ResultMapList response;
    m_playerProxy->getCurrentPlayQueue(callStatus, response, error);
    Common::BrowserMapListToJSON(response, result);
    return 0;
}
int CAPIClientPlayer::enqueueUri (json_t *json_params,  json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::mediamanager::PlayerTypes::RepeatStatus response;

    json_t *p0 = json_array_get(json_params, 0);
    const char *uri   = json_string_value (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->enqueueUri (uri, callStatus, error);
    *result = json_string("");
}
int CAPIClientPlayer::dequeueIndex (json_t *json_params,  json_t **result, void *data) {
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::mediamanager::PlayerTypes::RepeatStatus response;

    json_t *p0 = json_array_get(json_params, 0);
    json_int_t idx   = json_integer_value (p0);

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->dequeueIndex(idx, callStatus, error);
    *result = json_string("");
}
int CAPIClientPlayer::dequeueAll (json_t *json_params,  json_t **result, void *data) {
    org::genivi::mediamanager::PlayerTypes::PlayerError error;
    CommonAPI::CallStatus callStatus;
    org::genivi::mediamanager::PlayerTypes::RepeatStatus response;

    if (!m_playerProxy) {
        if (!initialize()) {
            std::cerr << "Failed to initialize CAPI client for indexer" << std::endl;
            return -1;
        }
    }

    m_playerProxy->dequeueAll(callStatus, error);
    *result = json_string("");
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

int capi_client_player_stop (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->stop(json_params, result, data);
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
int capi_client_player_getDuration (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getDuration(json_params, result, data);
}
int capi_client_player_getCurrentPlayQueue (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->getCurrentPlayQueue(json_params, result, data);
}
int capi_client_player_enqueueUri (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->enqueueUri(json_params, result, data);
}
int capi_client_player_dequeueIndex (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->dequeueIndex(json_params, result, data);
}
int capi_client_player_dequeueAll (json_t *json_params, json_t **result, void *data) {
    std::cout << "In method " << __FUNCTION__ << std::endl;
    CAPIClientPlayer *b = CAPIClientPlayer::instance();
    return b->dequeueAll(json_params, result, data);
}
