#include "../include/BlockManager.h"
#include "../include/config.h"
#include <iostream>

std::vector<int> BlockManager::freeBlocks;

void BlockManager::initializeFreeBlockList()
{
    for (int i = MAX_FILES / (BLOCK_SIZE / INODE_SIZE) + 1; i < NUM_BLOCKS; ++i)
    {
        freeBlocks.push_back(i);
    }
}

int BlockManager::allocateBlock()
{
    if (freeBlocks.empty())
    {
        std::cerr << "Error: No free blocks available." << std::endl;
        return -1;
    }
    int blockIndex = freeBlocks.back();
    freeBlocks.pop_back();
    return blockIndex;
}

void BlockManager::freeBlock(int blockIndex)
{
    freeBlocks.push_back(blockIndex);
}