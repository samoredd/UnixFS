#ifndef DIRECTORY_MANAGER_H
#define DIRECTORY_MANAGER_H

#include <unordered_map>
#include <fstream>
#include <string>

struct DirectoryEntry {
    char fileName[50];
    int inodeIndex;
    bool isDirectory;
};

class DirectoryManager {
public:
    static std::unordered_map<std::string, int> fileInodeMap;
    static std::fstream blockDevice;
    static void createDirectory(const std::string &dirName, int userId, int groupId);
    static void listDirectory(const std::string &dirName);
    static void addDirectoryEntry(int dirInodeIndex, const std::string &fileName, int fileInodeIndex, bool isDirectory);
};

#endif