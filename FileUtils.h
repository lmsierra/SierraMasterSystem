#pragma once

#include <stdio.h>

namespace FileUtils
{
    inline long GetFileSize(FILE* file)
    {
        const long current_position = ftell(file);

        fseek(file, 0, SEEK_END);
        const long size = ftell(file);

        fseek(file, current_position, 0);

        return size;
    }
};

