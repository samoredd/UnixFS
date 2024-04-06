#include "../include/DirectoryManager.h"
#include "../include/InodeManager.h"
#include "../include/BlockManager.h"
#include "../include/config.h"
#include <iostream>
#include <cstring>

std::unordered_map<std::string, int> DirectoryManager::fileInodeMap;
std::fstream DirectoryManager::blockDevice;

void DirectoryManager::createDirectory(const std::string &dirName, int userId, int groupId)
{
    int freeInodeIndex = InodeManager::allocateInode();
    if (freeInodeIndex == -1)
    {
        std::cerr << "Error: No free inode available." << std::endl;
        return;
    }

    Inode &inode = InodeManager::inodes[freeInodeIndex];
    inode.isDirectory = true;
    inode.size = 0;
    inode.creationTime = time(nullptr);
    inode.modificationTime = inode.creationTime;
    inode.permissions = READ | WRITE | EXECUTE;
    inode.userId = userId;
    inode.groupId = groupId;

    fileInodeMap[dirName] = freeInodeIndex;

    std::cout << "Directory created: " << dirName << " (inode: " << freeInodeIndex << ")" << std::endl;
}

void DirectoryManager::listDirectory(const std::string &dirName)
{
    int dirInodeIndex = fileInodeMap[dirName];
    Inode &dirInode = InodeManager::inodes[dirInodeIndex];

    std::cout << "Listing for directory: " << dirName << std::endl;
    for (int blockIndex : dirInode.directBlocks)
    {
        if (blockIndex == -1)
            break;
        DirectoryEntry entries[BLOCK_SIZE / sizeof(DirectoryEntry)];
        blockDevice.seekg(BLOCK_SIZE * blockIndex);
        blockDevice.read(reinterpret_cast<char *>(entries), sizeof(entries));

        for (const auto &entry : entries)
        {
            if (entry.inodeIndex != -1)
            {
                std::cout << entry.fileName << " (inode: " << entry.inodeIndex << ", directory: " << entry.isDirectory << ")" << std::endl;
            }
        }
    }
}

void DirectoryManager::addDirectoryEntry(int dirInodeIndex, const std::string &fileName, int fileInodeIndex, bool isDirectory)
{
    Inode &dirInode = InodeManager::inodes[dirInodeIndex];

    for (int blockIndex : dirInode.directBlocks)
    {
        if (blockIndex == -1)
            break;
        DirectoryEntry entries[BLOCK_SIZE / sizeof(DirectoryEntry)];
        blockDevice.seekg(BLOCK_SIZE * blockIndex);
        blockDevice.read(reinterpret_cast<char *>(entries), sizeof(entries));

        for (int i = 0; i < BLOCK_SIZE / sizeof(DirectoryEntry); ++i)
        {
            if (entries[i].inodeIndex == -1)
            {
                strncpy(entries[i].fileName, fileName.c_str(), sizeof(entries[i].fileName) - 1);
                entries[i].fileName[sizeof(entries[i].fileName) - 1] = '\0';
                entries[i].inodeIndex = fileInodeIndex;
                entries[i].isDirectory = isDirectory;

                blockDevice.seekp(BLOCK_SIZE * blockIndex);
                blockDevice.write(reinterpret_cast<char *>(entries), sizeof(entries));
                return;
            }
        }
    }

    int newBlockIndex = BlockManager::allocateBlock();
    if (newBlockIndex == -1)
    {
        std::cerr << "Error: Failed to allocate a block." << std::endl;
        return;
    }
    dirInode.directBlocks[dirInode.size / BLOCK_SIZE] = newBlockIndex;
    dirInode.size += BLOCK_SIZE;

    DirectoryEntry newEntry;
    strncpy(newEntry.fileName, fileName.c_str(), sizeof(newEntry.fileName) - 1);
    newEntry.fileName[sizeof(newEntry.fileName) - 1] = '\0';
    newEntry.inodeIndex = fileInodeIndex;
    newEntry.isDirectory = isDirectory;

    blockDevice.seekp(BLOCK_SIZE * newBlockIndex);
    blockDevice.write(reinterpret_cast<char *>(&newEntry), sizeof(DirectoryEntry));
}