#include "../include/FileManager.h"
#include "../include/InodeManager.h"
#include "../include/BlockManager.h"
#include "../include/DirectoryManager.h"
#include "../include/config.h"
#include <iostream>
#include <vector>
#include <cstring>

void FileManager::createFile(const std::string &fileName, const char *data, size_t dataSize, int userId, int groupId)
{
    int freeInodeIndex = InodeManager::allocateInode();
    if (freeInodeIndex == -1)
    {
        std::cerr << "Error: No free inode available." << std::endl;
        return;
    }

    Inode &inode = InodeManager::inodes[freeInodeIndex];
    inode.isDirectory = false;
    inode.size = dataSize;
    inode.creationTime = time(nullptr);
    inode.modificationTime = inode.creationTime;
    inode.permissions = READ | WRITE;
    inode.userId = userId;
    inode.groupId = groupId;

    int remainingSize = dataSize;
    int blockCounter = 0;

    while (remainingSize > 0 && blockCounter < 10)
    {
        int blockIndex = BlockManager::allocateBlock();
        if (blockIndex == -1)
        {
            std::cerr << "Error: Failed to allocate a block." << std::endl;
            InodeManager::freeInode(freeInodeIndex);
            return;
        }
        int writeSize = std::min(remainingSize, BLOCK_SIZE);
        DirectoryManager::blockDevice.seekp(BLOCK_SIZE * blockIndex);
        DirectoryManager::blockDevice.write(data + (dataSize - remainingSize), writeSize);

        inode.directBlocks[blockCounter++] = blockIndex;
        remainingSize -= writeSize;
    }

    if (remainingSize > 0)
    {
        inode.indirectBlock = BlockManager::allocateBlock();
        if (inode.indirectBlock == -1)
        {
            std::cerr << "Error: Failed to allocate an indirect block." << std::endl;
            InodeManager::freeInode(freeInodeIndex);
            return;
        }

        int *indirectBlockPointers = new int[BLOCK_SIZE / sizeof(int)];
        int indirectBlockIndex = 0;

        while (remainingSize > 0)
        {
            int blockIndex = BlockManager::allocateBlock();
            if (blockIndex == -1)
            {
                std::cerr << "Error: Failed to allocate a block." << std::endl;
                InodeManager::freeInode(freeInodeIndex);
                BlockManager::freeBlock(inode.indirectBlock);
                delete[] indirectBlockPointers;
                return;
            }
            int writeSize = std::min(remainingSize, BLOCK_SIZE);
            DirectoryManager::blockDevice.seekp(BLOCK_SIZE * blockIndex);
            DirectoryManager::blockDevice.write(data + (dataSize - remainingSize), writeSize);

            indirectBlockPointers[indirectBlockIndex++] = blockIndex;
            remainingSize -= writeSize;
        }

        DirectoryManager::blockDevice.seekp(BLOCK_SIZE * inode.indirectBlock);
        DirectoryManager::blockDevice.write(reinterpret_cast<char *>(indirectBlockPointers), indirectBlockIndex * sizeof(int));
        delete[] indirectBlockPointers;
    }

    DirectoryManager::fileInodeMap[fileName] = freeInodeIndex;

    std::cout << "File created: " << fileName << " (inode: " << freeInodeIndex << ")" << std::endl;
}

void FileManager::readFile(const std::string &fileName)
{
    if (DirectoryManager::fileInodeMap.find(fileName) == DirectoryManager::fileInodeMap.end())
    {
        std::cerr << "Error: File not found." << std::endl;
        return;
    }

    int inodeIndex = DirectoryManager::fileInodeMap[fileName];
    Inode &inode = InodeManager::inodes[inodeIndex];

    if (inode.size == 0)
    {
        std::cout << "File is empty." << std::endl;
        return;
    }

    std::vector<char> buffer(inode.size);
    char *bufferPtr = &buffer[0];
    int remainingSize = inode.size;

    for (int i = 0; i < 10 && inode.directBlocks[i] != -1 && remainingSize > 0; ++i)
    {
        int dataBlockIndex = inode.directBlocks[i];
        int readSize = std::min(remainingSize, BLOCK_SIZE);
        DirectoryManager::blockDevice.seekg(BLOCK_SIZE * dataBlockIndex);
        DirectoryManager::blockDevice.read(bufferPtr, readSize);

        bufferPtr += readSize;
        remainingSize -= readSize;
    }

    if (inode.indirectBlock != -1 && remainingSize > 0)
    {
        int *indirectBlockPointers = new int[BLOCK_SIZE / sizeof(int)];
        DirectoryManager::blockDevice.seekg(BLOCK_SIZE * inode.indirectBlock);
        DirectoryManager::blockDevice.read(reinterpret_cast<char *>(indirectBlockPointers), BLOCK_SIZE);

        for (int i = 0; indirectBlockPointers[i] != -1 && remainingSize > 0; ++i)
        {
            int dataBlockIndex = indirectBlockPointers[i];
            int readSize = std::min(remainingSize, BLOCK_SIZE);
            DirectoryManager::blockDevice.seekg(BLOCK_SIZE * dataBlockIndex);
            DirectoryManager::blockDevice.read(bufferPtr, readSize);

            bufferPtr += readSize;
            remainingSize -= readSize;
        }

        delete[] indirectBlockPointers;
    }

    std::cout << "File content: " << std::string(buffer.begin(), buffer.end()) << std::endl;
}

void FileManager::deleteFile(const std::string &fileName)
{
    if (DirectoryManager::fileInodeMap.find(fileName) == DirectoryManager::fileInodeMap.end())
    {
        std::cerr << "Error: File not found." << std::endl;
        return;
    }

    int inodeIndex = DirectoryManager::fileInodeMap[fileName];
    Inode &inode = InodeManager::inodes[inodeIndex];

    for (int i = 0; i < sizeof(inode.directBlocks) / sizeof(int); ++i)
    {
        if (inode.directBlocks[i] != -1)
        {
            BlockManager::freeBlock(inode.directBlocks[i]);
            inode.directBlocks[i] = -1;
        }
    }

    if (inode.indirectBlock != -1)
    {
        int *indirectBlockPointers = new int[BLOCK_SIZE / sizeof(int)];
        DirectoryManager::blockDevice.seekg(BLOCK_SIZE * inode.indirectBlock);
        DirectoryManager::blockDevice.read(reinterpret_cast<char *>(indirectBlockPointers), BLOCK_SIZE);

        for (int i = 0; indirectBlockPointers[i] != -1; ++i)
        {
            BlockManager::freeBlock(indirectBlockPointers[i]);
        }

        BlockManager::freeBlock(inode.indirectBlock);
        delete[] indirectBlockPointers;
    }

    InodeManager::freeInode(inodeIndex);
    DirectoryManager::fileInodeMap.erase(fileName);

    std::cout << "File deleted: " << fileName << std::endl;
}