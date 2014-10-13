#ifndef RAW_MESSAGE_INTERFACE_H
#define RAW_MESSAGE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif
    /**
     * Send a raw (as in not JSON) message to the CrossWalk application
     *
     * @param instance the CrossWalk instance which the message should be sent
     * to
     * @param message string of the message to be sent
     */
    void sendRawMessage(XW_Instance instance, const char *message);
#ifdef __cplusplus
}
#endif

#endif /* RAW_MESSAGE_INTERFACE_H */
