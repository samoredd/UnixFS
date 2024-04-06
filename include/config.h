const int BLOCK_SIZE = 1024;
const int NUM_BLOCKS = 1024;
const int INODE_SIZE = 128;
const int MAX_FILES = 50;

enum FilePermissions {
    READ = 0x1,
    WRITE = 0x2,
    EXECUTE = 0x4
};