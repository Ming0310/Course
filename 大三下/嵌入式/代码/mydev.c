#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define mydev_SIZE  0x1000
#define MEM_CLEAR 0x1  /* 清空内存的ioctl命令 */
#define mydev_MAJOR 230  /* 主设备号 */
#define mydev_MINOR 0
 static int mydev_major = mydev_MAJOR;
 static int mydev_minor = mydev_MINOR;
 module_param(mydev_major, int, S_IRUGO);

 /*
 * 这里借用了面向对象程序设计中的“封装”思想，把4KB内存和cdev包含在一个结构体中
 * 不过，你也可以不这么做：）
 */
 struct mydev_dev {
     struct cdev cdev;
     unsigned char mem[mydev_SIZE];  /* 申请4KB大小的内存 */
 };

struct mydev_dev *mydev_devp;



 /* 读设备
 * filp：文件结构体指针
 * buf：用户空间内存的地址，该地址在内核空间不能直接读写
 * size：要读的字节数
 * ppos：读的位置相对于文件开头的偏移
 */
ssize_t mydev_read(struct file *filp, char __user * buf, size_t size,loff_t * ppos){
    long p = *ppos;
    if(p >= mydev_SIZE)
        return 0;
    if(p + size > mydev_SIZE)
        size = mydev_SIZE - p;
    if(copy_to_user(buf,mydev_devp->mem+p,size)){
        return -1;
    }
    *ppos += size;
    return size;
}

ssize_t mydev_write(struct file *filp, const char __user * buf,size_t size, loff_t * ppos){
    long p = *ppos;
    if(p >= mydev_SIZE)
        return 0;
    if(p + size > mydev_SIZE)
        size = mydev_SIZE - p;
    if(copy_from_user(mydev_devp->mem+p,buf,size)){
        return -1;
    }
    *ppos += size;
    return size;
}

loff_t mydev_llseek(struct file *filp, loff_t off,int orig){
    loff_t NewPos=0;
    int offset=off;
    switch(orig){
        case SEEK_SET:    //SEEK_SET代表以文件头为偏移起始值
            NewPos=offset;
        break;
        case SEEK_CUR:    //SEEK_CUP代表以当前位置为偏移起始值
            NewPos=filp->f_pos+offset;
        break;
        case SEEK_END:    //SEEK_END代表以文件结尾为偏移起始值
            NewPos=mydev_SIZE+offset;
        break;
        default:
            return -1;
    }
    if(NewPos<0)
        return -1;
    filp->f_pos=NewPos;
    return NewPos;
}

 /* ioctl函数
 * filp：文件结构体指针
 * cmd：事先定义的I/O控制命令
 * arg：对应该命令的参数
 */
long mydev_ioctl(struct file *filp, unsigned int cmd,unsigned long arg)
 {

    switch (cmd) {
    case MEM_CLEAR:
    //clear mem
        memset(mydev_devp->mem,0,mydev_SIZE);
        break;

     default: /* 不能支持的命令 */
        return -1;
    }
    return 0;
 }

int mydev_open(struct inode* inode,struct file* filp){
    return 0;
}

int mydev_release(struct inode* inode,struct file* filp){
    return 0;
}


 struct file_operations mydev_fops = {
    .owner = THIS_MODULE,
    .llseek = mydev_llseek,
    .read = mydev_read,
    .write = mydev_write,
    .unlocked_ioctl = mydev_ioctl,
    .open = mydev_open,
    .release = mydev_release
};

static int __init mydev_init(void)
{
    int ret,err;
    dev_t devno = MKDEV(mydev_major, mydev_minor);

   /* 初始化cdev */
    mydev_devp = kzalloc(sizeof(struct mydev_dev), GFP_KERNEL);
    cdev_init(&mydev_devp->cdev, &mydev_fops);
    mydev_devp->cdev.owner = THIS_MODULE;

   /* 获取字符设备号 */
    if (mydev_major)
        ret = register_chrdev_region(devno, 1, "mydev");
    else {
        ret = alloc_chrdev_region(&devno, 0, 1, "mydev");
        mydev_major = MAJOR(devno);
        mydev_minor = MINOR(devno);
    }
    if (ret < 0)
        return ret;

     /* 注册设备 */
    err = cdev_add(&mydev_devp->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding mydev",err);
    return 1;
 }

 /* Linux设备加载函数 */
module_init(mydev_init);

static void __exit mydev_exit(void)
{
   /* 释放占用的设备号 */
    cdev_del(&mydev_devp->cdev);
    kfree(mydev_devp);
   /* 注销设备 */
    unregister_chrdev_region(MKDEV(mydev_major, mydev_minor), 1);
}

 /* Linux设备卸载函数 */
module_exit(mydev_exit);

MODULE_AUTHOR("ming");
MODULE_LICENSE("GPL v2");