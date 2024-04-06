#ifndef INODE_MANAGER_H
#define INODE_MANAGER_H

#include <vector>

struct Inode {
    int id;
    bool isDirectory;
    int size;
    int directBlocks[10];
    int indirectBlock;
    time_t creationTime;
    time_t modificationTime;
    int permissions;
    int userId;
    int groupId;
};

class InodeManager {
public:
    static std::vector<Inode> inodes;
    static int allocateInode();
    static void freeInode(int inodeIndex);
};

#endif