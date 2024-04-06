#include "../include/InodeManager.h"
#include "../include/BlockManager.h"
#include "../include/DirectoryManager.h"
#include "../include/FileManager.h"
#include "../include/config.h"
#include "../include/SuperBlock.h"
#include <iostream>
#include <string>
#include <mutex>

std::mutex fileSystemMutex;

void formatDisk() {
    DirectoryManager::blockDevice.open("fs.img", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

    if (!DirectoryManager::blockDevice) {
        std::cerr << "Error: Failed to open the block device file." << std::endl;
        return;
    }

    SuperBlock superBlock;
    superBlock.numInodes = MAX_FILES;
    superBlock.numBlocks = NUM_BLOCKS;
    superBlock.blockSize = BLOCK_SIZE;

    DirectoryManager::blockDevice.write(reinterpret_cast<char *>(&superBlock), sizeof(SuperBlock));

    InodeManager::inodes.resize(MAX_FILES);
    for (int i = 0; i < MAX_FILES; ++i) {
        Inode inode;
        inode.id = i;
        inode.isDirectory = false;
        inode.size = 0;
        std::memset(inode.directBlocks, -1, sizeof(inode.directBlocks));
        inode.indirectBlock = -1;
        inode.creationTime = 0;
        inode.modificationTime = 0;
        inode.permissions = 0;
        inode.userId = 0;
        inode.groupId = 0;
        InodeManager::inodes[i] = inode;

        DirectoryManager::blockDevice.write(reinterpret_cast<char *>(&inode), sizeof(Inode));
    }

    BlockManager::initializeFreeBlockList();

    std::cout << "Disk formatted successfully." << std::endl;
}

int main() {
    formatDisk();

    const char *data = "Hello, World!";
    FileManager::createFile("test.txt", data, strlen(data), 0, 0);
    FileManager::readFile("test.txt");

    // Test case 1: Creating a file
    std::string fileName1 = "file1.txt";
    const char *data1 = "This is the content of file1.";
    size_t dataSize1 = strlen(data1);
    fileSystemMutex.lock();
    FileManager::createFile(fileName1, data1, dataSize1, 0, 0);
    fileSystemMutex.unlock();

    // Test case 2: Creating a directory
    std::string dirName = "directory1";
    fileSystemMutex.lock();
    DirectoryManager::createDirectory(dirName, 0, 0);
    fileSystemMutex.unlock();

    // Test case 3: Creating a file with large data
    std::string fileName3 = "file2.txt";
    std::string data2 = "This is a large file with more than 10 blocks of data.";
    size_t dataSize2 = data2.length();
    fileSystemMutex.lock();
    FileManager::createFile(fileName3, data2.c_str(), dataSize2, 0, 0);
    fileSystemMutex.unlock();

    // Test case 4: Creating a file when no free inode is available
    for (int i = 0; i < MAX_FILES; ++i) {
        std::string fileName = "file" + std::to_string(i) + ".txt";
        std::string data = "This is the content of file" + std::to_string(i) + ".";
        size_t dataSize = data.length();
        fileSystemMutex.lock();
        FileManager::createFile(fileName, data.c_str(), dataSize, 0, 0);
        fileSystemMutex.unlock();
    }

    // Test case 5: Listing a directory
    fileSystemMutex.lock();
    DirectoryManager::addDirectoryEntry(DirectoryManager::fileInodeMap[dirName], "file1.txt", DirectoryManager::fileInodeMap[fileName1], false);
    DirectoryManager::addDirectoryEntry(DirectoryManager::fileInodeMap[dirName], "subdir", DirectoryManager::fileInodeMap[dirName], true);
    DirectoryManager::listDirectory(dirName);
    fileSystemMutex.unlock();

    // Test case 6: Deleting a file
    fileSystemMutex.lock();
    FileManager::deleteFile(fileName1);
    fileSystemMutex.unlock();

    DirectoryManager::blockDevice.close();
    return 0;
}