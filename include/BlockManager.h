#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include <vector>

class BlockManager {
public:
    static std::vector<int> freeBlocks;
    static void initializeFreeBlockList();
    static int allocateBlock();
    static void freeBlock(int blockIndex);
};

#endif