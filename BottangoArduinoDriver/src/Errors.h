
#ifndef BOTTANGO_ERRORS_H
#define BOTTANGO_ERRORS_H

namespace Error
{
    /** User tried to perform an operation on a pin with no servo registered to it */
    void reportError_NoServoOnPin();

    /** User tried to register a servo with a pin already reigistered */
    void reportError_ServoCollision();

    /** We ran out of space */
    void reportError_NoSpaceAvailable();

    /** User tried to send a command that exceeded the max length */
    void reportError_CmdTooLong();

    /** We registered too many command handlers */
    void reportError_TooManyCommands();

    /** Command had too many paramaters */
    void reportError_TooManyParams();

    /** Registering too many i2c drivers */
    void reportError_TooManyI2c();

    /** register an effector without including the required library */
    void reportError_MissingLibrary();

    /** regtister an effector without including the required library */
    void reportError_InvalidPin();

} // namespace Error

#endif // BOTTANGO_ERRORS_H
