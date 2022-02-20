#include "nfs.h"

/******************************************************************************
* SECTION: 宏定义
*******************************************************************************/
#define OPTION(t, p)        { t, offsetof(struct custom_options, p), 1 }

/******************************************************************************
* SECTION: 全局变量
*******************************************************************************/
static const struct fuse_opt option_spec[] = {		/* 用于FUSE文件系统解析参数 */
	OPTION("--device=%s", device),
	FUSE_OPT_END
};

struct custom_options nfs_options;			 /* 全局选项 */
struct nfs_super super;
/******************************************************************************
* SECTION: FUSE操作定义
*******************************************************************************/
static struct fuse_operations operations = {
	.init = nfs_init,						 /* mount文件系统 */		
	.destroy = nfs_destroy,				 /* umount文件系统 */
	.mkdir = nfs_mkdir,					 /* 建目录，mkdir */
	.getattr = nfs_getattr,				 /* 获取文件属性，类似stat，必须完成 */
	.readdir = nfs_readdir,				 /* 填充dentrys */
	.mknod = nfs_mknod,					 /* 创建文件，touch相关 */
	.write = NULL,								  	 /* 写入文件 */
	.read = NULL,								  	 /* 读文件 */
	.utimens = nfs_utimens,				 /* 修改时间，忽略，避免touch报错 */
	.truncate = NULL,						  		 /* 改变文件大小 */
	.unlink = NULL,							  		 /* 删除文件 */
	.rmdir	= NULL,							  		 /* 删除目录， rm -r */
	.rename = NULL,							  		 /* 重命名，mv */

	.open = NULL,							
	.opendir = NULL,
	.access = NULL
};
/******************************************************************************
* SECTION: 必做函数实现
*******************************************************************************/
/**
 * @brief 挂载（mount）文件系统
 * 
 * @param conn_info 可忽略，一些建立连接相关的信息 
 * @return void*
 * 结构：superblock | inode_map | data_map | inode | data
 */
void* nfs_init(struct fuse_conn_info * conn_info) {
	/* TODO: 在这里进行挂载 */
	nfs_mount();
	return NULL;
}

/**
 * @brief 卸载（umount）文件系统
 * 
 * @param p 可忽略
 * @return void
 */
void nfs_destroy(void* p) {
	/* TODO: 在这里进行卸载 */
	
	nfs_umount();

	return;
}

/**
 * @brief 创建目录
 * 
 * @param path 相对于挂载点的路径
 * @param mode 创建模式（只读？只写？），可忽略
 * @return int 0成功，否则失败
 */
int nfs_mkdir(const char* path, mode_t mode) {
	/* TODO: 解析路径，创建目录 */
	printf("创建目录：%s···\n",path);
	int find,root;
	find = root = 0;
	// 已存在
	nfs_look_up(path,&find,&root);
	if(find) return -NFS_ERROR_EXISTS;
	find = root = 0;
	//获取上一级path和新建的目录名称
	char* path_r,*fname;
	if(nfs_calc_lvl(path) == 1){
		path_r = (char*)malloc(sizeof(char) * 2);
		path_r[0] = '/';
		path_r[1] = '\0';
	}
	else{
		int i = strlen(path) - 1;
		for(;i >= 0;i--){
			if(path[i] == '/')
			break;
		}
		path_r = (char*)malloc(sizeof(char) * (i+1));
		path_r[i] = '\0';
		strncpy(path_r,path,i);
	}
	fname = strrchr(path,'/') + 1;

	struct nfs_inode* inode = nfs_look_up(path_r,&find,&root);
	if(!find)
	{
		return -NFS_ERROR_EXISTS;
	}
	if(inode->ftype == NFS_FILE){
		return -NFS_ERROR_UNSUPPORTED;
	}
	if(inode->size + sizeof(struct dentry_d) > NFS_BLK_PER_FILE * super.io_sz)
	{
		return -NFS_ERROR_NOSPACE;
	}
	// 将新建的目录项插入到上级目录的dentry下
	struct nfs_dentry* dentry_new = new_dentry(fname,NFS_DIR);
	struct nfs_inode* inode_new = nfs_new_inode(dentry_new);
	dentry_new->brother = inode->dentrys;
	inode->dentrys = dentry_new;
	inode->dir_cnt++;
	inode->size += sizeof(struct dentry_d);
	// 写回磁盘
	nfs_write_inode(inode);
	// struct nfs_inode* test_inode = nfs_read_inode(2,super.root_dentry);
	nfs_write_inode(inode_new);
	return 0;
}

/**
 * @brief 获取文件或目录的属性，该函数非常重要
 * 
 * @param path 相对于挂载点的路径
 * @param nfs_stat 返回状态
 * @return int 0成功，否则失败
 */
int nfs_getattr(const char* path, struct stat * nfs_stat) {
	/* TODO: 解析路径，获取Inode，填充nfs_stat，可参考/fs/simplefs/sfs.c的sfs_getattr()函数实现 */
	printf("getattr:%s\n",path);
	int find,root;
	find = root = 0;
	struct nfs_inode* inode= nfs_look_up(path,&find,&root);
	if(inode == NULL)return -NFS_ERROR_NOTFOUND;
	if(inode->ftype == NFS_FILE){
		nfs_stat->st_mode = NFS_DEFAULT_PERM | S_IFREG;
		nfs_stat->st_size = inode->size;
	}
	else if(inode->ftype == NFS_DIR){
		nfs_stat->st_mode = NFS_DEFAULT_PERM | S_IFDIR;
		nfs_stat->st_size = inode->dir_cnt * sizeof(struct dentry_d);
	}
	nfs_stat->st_nlink = 1;
	nfs_stat->st_uid = getuid();
	nfs_stat->st_gid = getuid();
	nfs_stat->st_atime   = time(NULL);
    nfs_stat->st_mtime   = time(NULL);
    nfs_stat->st_blksize = super.io_sz * 2;

	if(root){
		nfs_stat->st_size = super.use_sz * 2 * super.io_sz;
		nfs_stat->st_nlink = 2;
		nfs_stat->st_blocks = super.disk_sz / 2 / super.io_sz;
	}

	return 0;
}

/**
 * @brief 遍历目录项，填充至buf，并交给FUSE输出
 * 
 * @param path 相对于挂载点的路径
 * @param buf 输出buffer
 * @param filler 参数讲解:
 * 
 * typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
 *				const struct stat *stbuf, off_t off)
 * buf: name会被复制到buf中
 * name: dentry名字
 * stbuf: 文件状态，可忽略
 * off: 下一次offset从哪里开始，这里可以理解为第几个dentry
 * 
 * @param offset 第几个目录项？
 * @param fi 可忽略
 * @return int 0成功，否则失败
 */
int nfs_readdir(const char * path, void * buf, fuse_fill_dir_t filler, off_t offset,
			    		 struct fuse_file_info * fi) {
    /* TODO: 解析路径，获取目录的Inode，并读取目录项，利用filler填充到buf，可参考/fs/simplefs/sfs.c的sfs_readdir()函数实现 */
	printf("解析路径%s···\n",path);
	int find,root;
	find = root = 0;
	struct nfs_inode* inode = nfs_look_up(path,&find,&root);
	struct nfs_dentry* dentry;
	if(find){
		int cnt = 0;
		dentry = inode->dentrys;
		while(dentry){
			if(cnt == offset){
				filler(buf,dentry->name,NULL,++offset);
				return 0;
			}
			cnt++;
			dentry = dentry->brother;
		}
		return 0;
	}
    return -NFS_ERROR_NOTFOUND;
}

/**
 * @brief 创建文件
 * 
 * @param path 相对于挂载点的路径
 * @param mode 创建文件的模式，可忽略
 * @param dev 设备类型，可忽略
 * @return int 0成功，否则失败
 */
int nfs_mknod(const char* path, mode_t mode, dev_t dev) {
	/* TODO: 解析路径，并创建相应的文件 */
		printf("创建文件：%s···\n",path);
	int find,root;
	find = root = 0;
		// 已存在
	if(nfs_look_up(path,&find,&root))return -NFS_ERROR_EXISTS;
	find = root = 0;
	//获取上一级path和新建的目录名称
	char* path_r,*fname;
	if(nfs_calc_lvl(path) == 1){
		path_r = (char*)malloc(sizeof(char) * 2);
		path_r[0] = '/';
		path_r[1] = '\0';
	}
	else{
		int i = strlen(path) - 1;
		for(;i >= 0;i--){
			if(path[i] == '/')
			break;
		}
		path_r = (char*)malloc(sizeof(char) * (i+1));
		path_r[i] = '\0';
		strncpy(path_r,path,i);
	}
	fname = strrchr(path,'/') + 1;

	struct nfs_inode* inode = nfs_look_up(path_r,&find,&root);
	if(find == 0)
	{
		return -5;
	}
	if(inode->ftype == NFS_FILE){
		return -NFS_ERROR_UNSUPPORTED;
	}
	if(inode->size + sizeof(struct dentry_d) > NFS_BLK_PER_FILE * super.io_sz)
	{
		return -NFS_ERROR_NOSPACE;
	}
	// 将新建的目录项插入到上级目录的dentry下
	struct nfs_dentry* dentry_new = new_dentry(fname,NFS_FILE);
	struct nfs_inode* inode_new = nfs_new_inode(dentry_new);
	dentry_new->brother = inode->dentrys;
	inode->dentrys = dentry_new;
	inode->size += sizeof(struct dentry_d);
	inode->dir_cnt++;
	// 写回磁盘
	nfs_write_inode(inode);
	// struct nfs_inode* test_inode = nfs_read_inode(2,super.root_dentry);
	nfs_write_inode(inode_new);
	return 0;
}

/**
 * @brief 修改时间，为了不让touch报错 
 * 
 * @param path 相对于挂载点的路径
 * @param tv 实践
 * @return int 0成功，否则失败
 */
int nfs_utimens(const char* path, const struct timespec tv[2]) {
	(void)path;
	return 0;
}

/******************************************************************************
* SECTION: 选做函数实现
*******************************************************************************/
/**
 * @brief 写入文件
 * 
 * @param path 相对于挂载点的路径
 * @param buf 写入的内容
 * @param size 写入的字节数
 * @param offset 相对文件的偏移
 * @param fi 可忽略
 * @return int 写入大小
 */
int nfs_write(const char* path, const char* buf, size_t size, off_t offset,
		        struct fuse_file_info* fi) {
	/* 选做 */
	return size;
}

/**
 * @brief 读取文件
 * 
 * @param path 相对于挂载点的路径
 * @param buf 读取的内容
 * @param size 读取的字节数
 * @param offset 相对文件的偏移
 * @param fi 可忽略
 * @return int 读取大小
 */
int nfs_read(const char* path, char* buf, size_t size, off_t offset,
		       struct fuse_file_info* fi) {
	/* 选做 */
	return size;			   
}

/**
 * @brief 删除文件
 * 
 * @param path 相对于挂载点的路径
 * @return int 0成功，否则失败
 */
int nfs_unlink(const char* path) {
	/* 选做 */
	return 0;
}

/**
 * @brief 删除目录
 * 
 * 一个可能的删除目录操作如下：
 * rm ./tests/mnt/j/ -r
 *  1) Step 1. rm ./tests/mnt/j/j
 *  2) Step 2. rm ./tests/mnt/j
 * 即，先删除最深层的文件，再删除目录文件本身
 * 
 * @param path 相对于挂载点的路径
 * @return int 0成功，否则失败
 */
int nfs_rmdir(const char* path) {
	/* 选做 */
	return 0;
}

/**
 * @brief 重命名文件 
 * 
 * @param from 源文件路径
 * @param to 目标文件路径
 * @return int 0成功，否则失败
 */
int nfs_rename(const char* from, const char* to) {
	/* 选做 */
	return 0;
}

/**
 * @brief 打开文件，可以在这里维护fi的信息，例如，fi->fh可以理解为一个64位指针，可以把自己想保存的数据结构
 * 保存在fh中
 * 
 * @param path 相对于挂载点的路径
 * @param fi 文件信息
 * @return int 0成功，否则失败
 */
int nfs_open(const char* path, struct fuse_file_info* fi) {
	/* 选做 */
	return 0;
}

/**
 * @brief 打开目录文件
 * 
 * @param path 相对于挂载点的路径
 * @param fi 文件信息
 * @return int 0成功，否则失败
 */
int nfs_opendir(const char* path, struct fuse_file_info* fi) {
	/* 选做 */
	return 0;
}

/**
 * @brief 改变文件大小
 * 
 * @param path 相对于挂载点的路径
 * @param offset 改变后文件大小
 * @return int 0成功，否则失败
 */
int nfs_truncate(const char* path, off_t offset) {
	/* 选做 */
	return 0;
}


/**
 * @brief 访问文件，因为读写文件时需要查看权限
 * 
 * @param path 相对于挂载点的路径
 * @param type 访问类别
 * R_OK: Test for read permission. 
 * W_OK: Test for write permission.
 * X_OK: Test for execute permission.
 * F_OK: Test for existence. 
 * 
 * @return int 0成功，否则失败
 */
int nfs_access(const char* path, int type) {
	/* 选做: 解析路径，判断是否存在 */
	return 0;
}	

int
nfs_mount(){
	// flag 标记是否格式化
	int flag = 0;
    struct super_block_d super_block_d;
	int map_data_number;
	int map_inode_number,super_blks;
	int inode_number;

	struct nfs_inode* root_inode;
	super.fd = ddriver_open(nfs_options.device);
	// 获取disk和io的size
	ddriver_ioctl(super.fd,IOC_REQ_DEVICE_SIZE,&super.disk_sz);
	ddriver_ioctl(super.fd,IOC_REQ_DEVICE_IO_SZ,&super.io_sz);
	// 申请根目录
	super.root_dentry = new_dentry("/",NFS_DIR);
	// 读取超级块的信息
	
	nfs_driver_read(0,(uint8_t*)(&super_block_d),sizeof(struct super_block_d));
	if(super_block_d.magic_num != NFS_MAGIC){
		// 需要重新载入，开始估算
		flag = 1;
		// 超级块占的磁盘块数
		super_blks = NFS_ROUND_UP(sizeof(struct super_block_d),super.io_sz) / super.io_sz;
		// inode位图和数据位图占的块数
		// total_number = super.disk_sz / (super.io_sz);
		inode_number = (super.disk_sz) / ((NFS_BLK_PER_FILE + 1) * super.io_sz);
		map_inode_number = NFS_ROUND_UP((inode_number / 8 + 1),super.io_sz) / super.io_sz;
		map_data_number = NFS_ROUND_UP(((inode_number * NFS_BLK_PER_FILE / 2) / 8 + 1),super.io_sz) / super.io_sz;
		// 总估计数 - superblk - datamap - inodemap = 最大inode数
		super_block_d.max_ino = inode_number - super_blks - map_data_number - map_inode_number;
		super.max_ino = super_block_d.max_ino;
		super_block_d.map_data_blks = map_data_number;
		super_block_d.map_inode_blks = map_inode_number;
		super_block_d.map_inode_offset = super_blks*super.io_sz;
		super_block_d.map_data_offset = super_block_d.map_inode_offset + super_block_d.map_inode_blks * super.io_sz;
		super_block_d.inode_offset = super_block_d.map_data_offset + super_block_d.map_data_blks * super.io_sz;
		super_block_d.data_offset = super_block_d.inode_offset + super_block_d.max_ino * super.io_sz;
		super_block_d.use_sz = 0;
		
		super_block_d.magic_num = NFS_MAGIC;
	}
	// 建立内存结构
	super.max_ino = super_block_d.max_ino;
	super.map_inode = (uint8_t*)malloc(super_block_d.map_inode_blks * super.io_sz);
	super.map_data = (uint8_t*)malloc(super_block_d.map_data_blks * super.io_sz);
	super.inode_offset = super_block_d.inode_offset;
	super.data_offset = super_block_d.data_offset;
	super.map_inode_blks = super_block_d.map_inode_blks;
	super.map_data_blks = super_block_d.map_data_blks;
	super.use_sz = super_block_d.use_sz;
	nfs_driver_read(super_block_d.map_inode_offset,super.map_inode,super_block_d.map_inode_blks * super.io_sz);
	nfs_driver_read(super_block_d.map_data_offset,super.map_data,super_block_d.map_data_blks * super.io_sz);

	if(flag){
		// superblk写回磁盘
		nfs_driver_write(0,(uint8_t*)(&super_block_d),sizeof(struct super_block_d));
		super.map_inode[0] = 192;
		nfs_driver_write(super_block_d.map_inode_offset,super.map_inode,super_block_d.map_inode_blks * super.io_sz);
		nfs_driver_write(super_block_d.map_data_offset,super.map_data,super_block_d.map_data_blks * super.io_sz);

		// 建立根节点并写回磁盘
		root_inode = nfs_new_inode(super.root_dentry);
		nfs_write_inode(root_inode);
	}
	else{
			super.root_dentry->ino = 2;
			nfs_read_inode(2,super.root_dentry);
	}

	return 0;
}

int
nfs_umount(){
	// 写回superblk
	struct super_block_d super_block_d;
	nfs_driver_read(0,(uint8_t*)(&super_block_d),sizeof(struct super_block_d));
	super_block_d.use_sz = super.use_sz;
	nfs_driver_write(0,(uint8_t*)(&super_block_d),sizeof(struct super_block_d));
	// 更新datamap和inodemap
	nfs_driver_write(super_block_d.map_inode_offset,super.map_inode,super_block_d.map_inode_blks * super.io_sz);
	nfs_driver_write(super_block_d.map_data_offset,super.map_data,super_block_d.map_data_blks * super.io_sz);
	ddriver_close(super.fd);

	return 0;
}



// 避免直接读磁盘时，偏移量没有对齐io_sz
int
nfs_driver_read(int offset,uint8_t* param,int size){
	// 按IO块对齐
	// debug
	// printf("offset:%d\n,super_iosz%d\n",offset,super.io_sz);
	
	int      offset_aligned = NFS_ROUND_DOWN(offset, super.io_sz);
    int      bias           = offset - offset_aligned;
    int      size_aligned   = NFS_ROUND_UP((size + bias), super.io_sz);
	uint8_t* temp = (uint8_t*)malloc(size_aligned);
	uint8_t* cur = temp;
	// 移动磁盘头
	ddriver_seek(super.fd,offset_aligned,SEEK_SET);
	while(size_aligned != 0){
		ddriver_read(super.fd,cur,super.io_sz);
		size_aligned -= super.io_sz;
		cur += super.io_sz;
	}
	memcpy(param,temp + bias,size);
	free(temp);
	return 0;
}

//	避免直接写磁盘时，偏移量没有对齐io_sz
int
nfs_driver_write(int offset,uint8_t* param,int size){
	int      offset_aligned = NFS_ROUND_DOWN(offset, super.io_sz);
    int      bias           = offset - offset_aligned;
    int      size_aligned   = NFS_ROUND_UP((size + bias), super.io_sz);
	uint8_t* temp = (uint8_t*)malloc(size_aligned);
	uint8_t* cur = temp;
	// 写操作也要整块写，故先读出来没修改的部分，覆盖掉要修改的部分
	nfs_driver_read(offset_aligned,temp,size_aligned);
	memcpy(temp + bias,param,size);
	ddriver_seek(super.fd,offset_aligned,SEEK_SET);
	while(size_aligned != 0){
		ddriver_write(super.fd,cur,super.io_sz);
		size_aligned -= super.io_sz;
		cur += super.io_sz;
	}
	free(temp);
	return 0;
}

// 申请一个新的inode
struct nfs_inode*
nfs_new_inode(struct nfs_dentry* dentry){
	struct nfs_inode* inode;
	int byte_cursor = 0; 
    int bit_cursor  = 0; 
    int ino_cursor  = 0;
	int data_cursor = 0;
	int flag = 0;
	for(;byte_cursor < super.map_inode_blks * super.io_sz;byte_cursor++){
		uint8_t* cur = 	super.map_inode + byte_cursor;
		for(bit_cursor = 0;bit_cursor < 8;bit_cursor++){
			if(((*cur) & (1 << (7 - bit_cursor))) == 0){
				(*cur) = (*cur) | (1 << (7 - bit_cursor));
				flag = 1;
				break;
			}
			ino_cursor++;
		}
		if(flag) break;
	}
	// 没有inode可用
	if(!flag || ino_cursor == super.max_ino) return NULL;
	inode = (struct nfs_inode*)malloc(sizeof(struct nfs_inode));
	inode->ino = ino_cursor;
	inode->dentry = dentry;
	inode->size = 0;
	inode->dir_cnt = 0;
	inode->ftype = dentry->type;
	dentry->ino = inode->ino;
	// 根据datamap找磁盘块
	int cnt = 0;
		for(byte_cursor = 0;byte_cursor < super.map_data_blks * super.io_sz;byte_cursor++){
		uint8_t* cur = 	super.map_data + byte_cursor;
		for(bit_cursor = 0;bit_cursor < 8;bit_cursor++){
			if(((*cur) & (1 << (7 - bit_cursor))) == 0){
				(*cur) = (*cur) | ((1 << (7 - bit_cursor)));
				inode->block_pointer[cnt++] = data_cursor;
				if(cnt == 6)
					break;
			}
			data_cursor++;
		}
		if(cnt == 6) break;
	}
	// 没有足够的datablk
	if(cnt != 6)return NULL;
	super.use_sz += 6;
	return inode;
}

// 将inode写入磁盘
int
nfs_write_inode(struct nfs_inode* inode){
	struct inode_d inode_d;
	struct dentry_d dentry_d;
	struct nfs_dentry* dentry_cursor;
	int offset;
	// 构建磁盘内存储的inode结构
	inode_d.ino = inode->ino;
	inode_d.dir_cnt = inode->dir_cnt;
	inode_d.ftype = inode->ftype;
	inode_d.size = inode->size;
	for (int i = 0; i < 6; i++)
	{
		inode_d.block_pointer[i] = inode->block_pointer[i];
	}
	//写inode
	nfs_driver_write(NFS_INO_OFF(inode_d.ino),(uint8_t*)(&inode_d),sizeof(inode_d));
	// 写inode指向的数据块
	// if(inode->ftype == NFS_FILE){
	// 	// 该inode所指的是文件
	// 	nfs_driver_write(NFS_INO_OFF(inode_d.ino),(uint8_t*)(&inode_d),sizeof(struct inode_d));
	// }
	 if(inode->ftype == NFS_DIR){
		// 该inode指向的是目录项
		offset = NFS_DATA_OFF(inode->block_pointer[0]);
		dentry_cursor = inode->dentrys;
		while(dentry_cursor != NULL){
			memcpy(dentry_d.fname,dentry_cursor->name,MAX_NAME_LEN);
			dentry_d.ftype = dentry_cursor->type;
			dentry_d.ino = dentry_cursor->ino;
			dentry_d.valid = dentry_cursor->valid;
			nfs_driver_write(offset,(uint8_t*)(&dentry_d),sizeof(struct dentry_d));
			dentry_cursor = dentry_cursor->brother;
			offset += sizeof(struct dentry_d);
		}
	 }

	return 0;
}


int 
nfs_calc_lvl(const char * path) {
    // char* path_cpy = (char *)malloc(strlen(path));
    // strcpy(path_cpy, path);
    char* str = path;
    int   lvl = 0;
    if (strcmp(path, "/") == 0) {
        return lvl;
    }
    while (*str != NULL) {
        if (*str == '/') {
            lvl++;
        }
        str++;
    }
    return lvl;
}


struct nfs_inode*
nfs_read_inode(int ino,struct nfs_dentry* dentry){
	struct inode_d inode_d;
	struct nfs_inode* inode_ret = (struct nfs_inode*)malloc(sizeof(struct nfs_inode));
	nfs_driver_read(NFS_INO_OFF(ino),(uint8_t*)(&inode_d),sizeof(inode_d));
	for(int i = 0;i < 6;i++)inode_ret->block_pointer[i] = inode_d.block_pointer[i];
	inode_ret->ino = ino;
	inode_ret->size = inode_d.size;
	inode_ret->ftype = inode_d.ftype;
	inode_ret->dir_cnt = inode_d.dir_cnt;
	inode_ret->dentry = dentry;
	inode_ret->dentrys = NULL;
	if(inode_d.ftype == NFS_DIR && inode_d.dir_cnt > 0){
		int offset;
		struct dentry_d dentry_d;
		offset = NFS_DATA_OFF(inode_d.block_pointer[0]);
		nfs_driver_read(offset,(uint8_t*)(&dentry_d),sizeof(struct dentry_d));
		struct nfs_dentry* dentry_cd = new_dentry(dentry_d.fname,dentry_d.ftype);
		dentry_cd->ino = dentry_d.ino;
		dentry_cd->valid = dentry_d.valid;
		inode_ret->dentrys = dentry_cd;
		offset += sizeof(struct dentry_d);
		for(int i = 1;i < inode_d.dir_cnt;i++){
			// int blk_num = (i) * sizeof(struct dentry_d) / (2 * super.io_sz);
			nfs_driver_read(offset,(uint8_t*)(&dentry_d),sizeof(struct dentry_d));
			struct nfs_dentry* dentry_cd1 = new_dentry(dentry_d.fname,dentry_d.ftype);
			dentry_cd1->ino = dentry_d.ino;
			dentry_cd1->valid = dentry_d.valid;
			dentry_cd->brother = dentry_cd1;
			dentry_cd = dentry_cd1;
			offset += sizeof(struct dentry_d);
		}
	}
	return inode_ret;
}


// 返回查找到的文件/目录的dentry
struct nfs_inode*
nfs_look_up(const char* path,int* find,int* root){
	int total_lv = nfs_calc_lvl(path);
	int flag = 0;
	(*find) = (*root) = 0;
	if(total_lv == 0){
		(*find) = (*root) = 1;
		struct nfs_inode* inode_ret = nfs_read_inode(super.root_dentry->ino,super.root_dentry);
		return inode_ret; 
	}
	struct nfs_dentry* dentry_cur,*dentry_ret;
	char* path_cp = (char*)malloc(sizeof(path));
	strcpy(path_cp,path);
	char* fname = strtok(path_cp,"/");
	dentry_cur = super.root_dentry;
	while(fname != NULL){
		total_lv--;
		struct nfs_inode* inode = nfs_read_inode(dentry_cur->ino,dentry_cur);
		dentry_cur = inode->dentrys;
		while(dentry_cur != NULL){
			if(memcmp(dentry_cur->name,fname,strlen(fname)) == 0){
				dentry_ret = dentry_cur;
				if(total_lv == 0){
					flag = 1;
				}
				else{
					fname = strtok(NULL,"/");
				}
				break;
			}
			else{
				dentry_cur = dentry_cur->brother;
			}
		}
		if(flag == 1)break;
		if(dentry_cur == NULL)return NULL;
	}
	if(flag){
		(*find) = 1;
		return nfs_read_inode(dentry_ret->ino,dentry_ret);
	}
	else
		return NULL;
}
/******************************************************************************
* SECTION: FUSE入口
*******************************************************************************/
int main(int argc, char **argv)
{
    int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	nfs_options.device = strdup("./tests/mnt");

	if (fuse_opt_parse(&args, &nfs_options, option_spec, NULL) == -1)
		return -1;
	
	ret = fuse_main(args.argc, args.argv, &operations, NULL);
	fuse_opt_free_args(&args);
	return ret;
}