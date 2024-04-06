#include "../include/config.h"
#include "../include/FileManager.h"
#include "../include/InodeManager.h"
#include "../include/BlockManager.h"
#include "../include/DirectoryManager.h"
#include "../include/SuperBlock.h"
#include <iostream>
#include <string>
#include <cstring>
#include <mutex>
#include <sstream>

std::mutex fileSystemMutex;

void formatDisk()
{
    DirectoryManager::blockDevice.open("fs.img", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

    if (!DirectoryManager::blockDevice)
    {
        std::cerr << "Error: Failed to open the block device file." << std::endl;
        return;
    }

    SuperBlock superBlock;
    superBlock.numInodes = MAX_FILES;
    superBlock.numBlocks = NUM_BLOCKS;
    superBlock.blockSize = BLOCK_SIZE;

    DirectoryManager::blockDevice.write(reinterpret_cast<char *>(&superBlock), sizeof(SuperBlock));

    InodeManager::inodes.resize(MAX_FILES);
    for (int i = 0; i < MAX_FILES; ++i)
    {
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

void displayHelp()
{
    std::cout << "Available commands:" << std::endl;
    std::cout << "  create <filename> <content>   Create a new file with the specified content" << std::endl;
    std::cout << "  read <filename>               Read the content of a file" << std::endl;
    std::cout << "  delete <filename>             Delete a file" << std::endl;
    std::cout << "  mkdir <directory>             Create a new directory" << std::endl;
    std::cout << "  ls <directory>                List the contents of a directory" << std::endl;
    std::cout << "  help                          Display this help message" << std::endl;
    std::cout << "  exit                          Exit the program" << std::endl;
}

int main(int argc, char *argv[])
{
    formatDisk();

    displayHelp();

    while (true)
    {
        std::cout << "\nEnter a command: ";
        std::string command;
        std::getline(std::cin, command);

        std::vector<std::string> tokens;
        std::istringstream iss(command);
        std::string token;
        while (std::getline(iss, token, ' '))
        {
            tokens.push_back(token);
        }

        if (tokens.empty())
        {
            continue;
        }

        std::string operation = tokens[0];

        if (operation == "create")
        {
            if (tokens.size() < 3)
            {
                std::cout << "Usage: create <filename> <content>" << std::endl;
                continue;
            }
            std::string fileName = tokens[1];
            std::string data = tokens[2];

            fileSystemMutex.lock();
            FileManager::createFile(fileName, data.c_str(), data.length(), 0, 0);
            fileSystemMutex.unlock();
        }
        else if (operation == "read")
        {
            if (tokens.size() < 2)
            {
                std::cout << "Usage: read <filename>" << std::endl;
                continue;
            }
            std::string fileName = tokens[1];

            fileSystemMutex.lock();
            FileManager::readFile(fileName);
            fileSystemMutex.unlock();
        }
        else if (operation == "delete")
        {
            if (tokens.size() < 2)
            {
                std::cout << "Usage: delete <filename>" << std::endl;
                continue;
            }
            std::string fileName = tokens[1];

            fileSystemMutex.lock();
            FileManager::deleteFile(fileName);
            fileSystemMutex.unlock();
        }
        else if (operation == "mkdir")
        {
            if (tokens.size() < 2)
            {
                std::cout << "Usage: mkdir <directory>" << std::endl;
                continue;
            }
            std::string dirName = tokens[1];

            fileSystemMutex.lock();
            DirectoryManager::createDirectory(dirName, 0, 0);
            fileSystemMutex.unlock();
        }
        else if (operation == "ls")
        {
            if (tokens.size() < 2)
            {
                std::cout << "Usage: ls <directory>" << std::endl;
                continue;
            }
            std::string dirName = tokens[1];

            fileSystemMutex.lock();
            DirectoryManager::listDirectory(dirName);
            fileSystemMutex.unlock();
        }
        else if (operation == "help")
        {
            displayHelp();
        }
        else if (operation == "exit")
        {
            break;
        }
        else
        {
            std::cout << "Invalid command. Type 'help' for available commands." << std::endl;
        }
    }

    DirectoryManager::blockDevice.close();
    return 0;
}