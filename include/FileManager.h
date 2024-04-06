#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>

class FileManager {
public:
    static void createFile(const std::string &fileName, const char *data, size_t dataSize, int userId, int groupId);
    static void readFile(const std::string &fileName);
    static void deleteFile(const std::string &fileName);
};

#endif