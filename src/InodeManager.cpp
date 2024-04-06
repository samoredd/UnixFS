#include "../include/InodeManager.h"
#include "../include/config.h"
#include <cstring>

std::vector<Inode> InodeManager::inodes;

int InodeManager::allocateInode() {
        for (int i = 0; i < MAX_FILES; ++i) {
            if (inodes[i].size == 0) {
                return i;
            }
        }
        return -1;
    }

void InodeManager::freeInode(int inodeIndex) {
        Inode &inode = inodes[inodeIndex];
        inode.size = 0;
        inode.isDirectory = false;
        std::memset(inode.directBlocks, -1, sizeof(inode.directBlocks));
        inode.indirectBlock = -1;
    }