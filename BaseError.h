/*
 * Author: Nadav Markus
 * The base class for my thrown errors.
 */

#ifndef __BASE_ERROR_H
#define __BASE_ERROR_H

#include <string>

class BaseError
{
private:
    const std::string message;
public:
    BaseError(const std::string &message): message(message) {}
    virtual const std::string& getMessage() const { return message; }
};

#endif