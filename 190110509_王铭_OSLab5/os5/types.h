#ifndef _TYPES_H_
#define _TYPES_H_

// 平均每个文件占用多少个io基本块
#define NFS_BLK_PER_FILE 12
#define MAX_NAME_LEN    128     
#define NFS_ROUND_DOWN(value, round)    (value % round == 0 ? value : (value / round) * round)
#define NFS_ROUND_UP(value, round)      (value % round == 0 ? value : (value / round + 1) * round)
#define NFS_INO_OFF(ino)                (super.inode_offset + ino * sizeof(struct inode_d))
#define NFS_DATA_OFF(blknum)                (super.data_offset + blknum * super.io_sz * 2)
#define NFS_DATA_BLK_SZ                    2*super.io_sz
#define NFS_DEFAULT_PERM 0777

#define NFS_ERROR_EXISTS        EEXIST
#define NFS_ERROR_UNSUPPORTED   ENXIO
#define NFS_ERROR_NOTFOUND      ENOENT
#define NFS_ERROR_NOSPACE       ENOSPC

struct custom_options {
	const char*        device;
};


typedef enum nfs_file_type{
    NFS_FILE,
    NFS_DIR
}NFS_FILE_TYPE;

//super block
struct nfs_super {
    int fd;         // 系统文件号
    int      max_ino; // 最大文件数,一个文件一个inode
    int     io_sz;  //io基本块大小
    int     disk_sz; // 磁盘大小
    int     map_inode_blks;// inode位图占的磁盘块大小
    int     map_data_blks;// data位图占的磁盘块大小
    int     inode_offset; // 索引的偏移量
    int     data_offset; // 数据块的偏移量
    int     use_sz; // 使用的数据块数,一个数据块 = 两个io基本块
    struct nfs_dentry*  root_dentry; //根目录
    uint8_t*    map_inode; // inode位图
    uint8_t*    map_data; // data位图
};

// inode
struct nfs_inode {
    uint32_t ino;       //下标
    int      size;                 // 文件已占用空间
    int      dir_cnt;              // 目录项数量
    NFS_FILE_TYPE ftype;
    struct nfs_dentry*  dentry;               // 指向该inode的dentry
    struct nfs_dentry*  dentrys;              // 所有目录项
    int      block_pointer[6];     // 数据块指针
};  

// 目录项
struct nfs_dentry {
    char     name[MAX_NAME_LEN];
    uint32_t ino;
    NFS_FILE_TYPE type;
    int      valid;
    struct nfs_dentry* brother;
};


static inline struct nfs_dentry* new_dentry(char * fname, NFS_FILE_TYPE ftype) {
    struct nfs_dentry * dentry = (struct nfs_dentry *)malloc(sizeof(struct nfs_dentry));
    memset(dentry, 0, sizeof(struct nfs_dentry));
    memcpy(dentry->name,fname,MAX_NAME_LEN);
    dentry->type = ftype;
    dentry->ino = -1;
    dentry->brother = NULL;
    dentry->valid = 1;
    return dentry;
}




// 实际物理存储

struct super_block_d
{
    uint32_t           magic_num;                   // 幻数

    int                max_ino;                     // 最多支持的文件数

    int                map_inode_blks;              // inode位图占用的块数
    int                map_inode_offset;            // inode位图在磁盘上的偏移

    int                map_data_blks;               // data位图占用的块数
    int                map_data_offset;             // data位图在磁盘上的偏移

    int                inode_offset;                // inode区偏移量
    int                data_offset;                 // 数据区偏移量
    int                 use_sz;                     // 使用的数量
};
struct inode_d
{
    int                ino;                // 在inode位图中的下标
    int                size;               // 文件已占用空间
    // int                link;               // 链接数
    NFS_FILE_TYPE          ftype;              // 文件类型（目录类型、普通文件类型）
    int                dir_cnt;            // 如果是目录类型文件，下面有几个目录项
    int                block_pointer[6];   // 数据块指针（可固定分配）
}; 

struct dentry_d
{
    char               fname[MAX_NAME_LEN];          // 指向的ino文件名
    NFS_FILE_TYPE          ftype;                         // 指向的ino文件类型
    int                ino;                           // 指向的ino号
    int                valid;                         // 该目录项是否有效
};

#endif /* _TYPES_H_ */