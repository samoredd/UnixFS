#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

struct SuperBlock {
    int numInodes;
    int numBlocks;
    int blockSize;
};

#endif