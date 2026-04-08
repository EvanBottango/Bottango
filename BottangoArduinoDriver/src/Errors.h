
#ifndef BOTTANGO_ERRORS_H
#define BOTTANGO_ERRORS_H

namespace Error
{
    /** User tried to perform an operation on an identifier with no effector registered to it */
    void reportError_NoEffectorOnPin(const char *identifer);

    /** User tried to register a servo with a pin already reigistered */
    void reportError_EffectorCollision(const char *identifer);

    /** We ran out of space */
    void reportError_NoSpaceAvailable();

#ifdef NAMED_BOARD
    void reportError_InvalidPin();
#endif

    /** User tried to send a command that exceeded the max length */
    void reportError_CmdTooLong(int length);

    /** Command had too many paramaters */
    void reportError_TooManyParams(int length);

    /** register an effector without including the required library */
    void reportError_MissingLibrary(const char *libName);

    /** multi-message response timed out waiting for a continue */
    void reportError_MultiMessageTimeout();

#ifdef RELAY_SUPPORTED
    void reportError_NoRelayForID(int id);
    void reportError_RelayCollision(byte mac[6]);
#endif

} // namespace Error

#endif // BOTTANGO_ERRORS_H
