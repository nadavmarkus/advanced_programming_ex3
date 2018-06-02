/*
 * Author: Nadav Markus
 * An error that gets thrown in case of a bad move.
 */

#ifndef __BAD_MOVE_ERROR_H_
#define __BAD_MOVE_ERROR_H_

class BadMoveError : public BaseError
{
public:
    BadMoveError(const std::string &message): BaseError(message) {}
};

#endif