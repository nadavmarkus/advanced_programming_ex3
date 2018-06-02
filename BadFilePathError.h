/*
 * Author: Nadav Markus
 * An error that gets thrown in case of non-found path or the like.
 */

#ifndef __BAD_FILE_ERROR_H_
#define __BAD_FILE_ERROR_H_

#include <string>

class BadFilePathError
{
    private:
        const std::string path;
    public:
        BadFilePathError(const std::string &path) : path(path) {}
};

#endif 