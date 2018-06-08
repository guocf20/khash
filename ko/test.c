/*---------------------
* Copyright(C)2018 All rights reserved
*author:guochengfeng
*last modify:2018--05--23
*email:guocf20@gmail.com
*=================================*/
#include <linux/init.h>  
#include <linux/module.h>  
#include<asm/uaccess.h>

#include <linux/kernel.h>  
#include <linux/fs.h>  
#include <linux/delay.h>  
#include <asm/uaccess.h>  
#include <asm/irq.h>  
#include <asm/io.h>  
#include <linux/device.h>  
#include<linux/ioctl.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/semaphore.h>
#include <linux/sched.h>

MODULE_LICENSE("Dual BSD/GPL");  

struct semaphore sem;
static int major = 130;

static struct class *firstdrv_class;  
static struct device *firstdrv_device; 
static char msg[128] = {'\0'};


#define MEM_IOC_MAGIC     'm'     
#define MEM_IOCSET     _IOW(MEM_IOC_MAGIC, 0, int) 
#define MEM_IOCGET     _IOR(MEM_IOC_MAGIC, 1, int)  

#define DEVICE_NAME "test"

#define IOCTL_GET 101
#define IOCTL_SET 102

#define MEMDEV_IOC_MAXNR     3



typedef struct
{
    char *key;
    char *value;
    struct list_head list;
}hashlist;


typedef struct
{
    int tsize;
    hashlist **lists;
}hashtbl;

hashtbl *tbl;

hashtbl *inithash(int size)
{
    int i = 0;
    hashtbl *tbl;
    tbl = kzalloc(sizeof(hashtbl), 0);
    if(NULL == tbl)
    {
        return NULL;
    }
    tbl->tsize = size;

    tbl->lists = kzalloc(sizeof(hashlist) * size, 0);
    for(i = 0 ; i < size;i++)
    {
        tbl->lists[i] = kzalloc(sizeof(hashlist), 0);
        if(NULL == tbl->lists[i])
        {
            return NULL;
        }
        else
        {
            INIT_LIST_HEAD(&tbl->lists[i]->list);
        }
    }
    return tbl;
}

unsigned int DJB_hash(const char *str)    
{    
    unsigned int hash = 5381;    
                         
    while (*str)    
    {    
        hash += (hash << 5) + (*str++);    
    }    
                                      
    return (hash & 0x7FFFFFFF);    
} 

static int hash(const char *str, int TableSize )
{
    unsigned int hval = DJB_hash(str);
    return (hval%TableSize);
}

static int find(const char *str, char *buf, hashtbl *tbl)
{
    int i =  hash(str, tbl->tsize);
    hashlist *p, *entry;
    p = tbl->lists[i];
    list_for_each_entry(entry, &p->list, list)
    {
        printk("in find key = %s, value = %s\n",entry->key, entry->value);
        if(strcmp(str, entry->key) == 0)
        {
            if(buf != NULL)
            {
                sprintf(buf, "%s", entry->value);
            }
            return 0;
        }
    }
    return 1;
}

static void  insert(const char *key, const char *value, hashtbl *tbl)
{
     int i = 0;
     hashlist *entry;
     hashlist *ele;
     
     int flag = find(key, NULL, tbl);
     if(flag == 0)
     {
         printk("find\n");
     }
     else
     {
         printk("not find\n");
     }
     i = hash(key, tbl->tsize);
     printk("hash = %d\n", i);

     entry = tbl->lists[i];

     ele = kzalloc(sizeof(hashlist), 0);
     ele->key = kzalloc(strlen(key) + 1, 0);
     strcpy(ele->key, key);
     ele->value = kzalloc(strlen(value) + 1, 0);
     strcpy(ele->value, value);
     list_add(&ele->list, &entry->list);

}

static void delete (const char *key, hashtbl *tbl)
{
     int i = 0;
     hashlist *p = NULL;
     hashlist *entry, *tmp;
     
     int flag = find(key, NULL, tbl);
     if(flag == 0)
     {
         printk("find\n");
     }
     else
     {
         printk("not find\n");
     }

     i = hash(key, tbl->tsize);
     printk("hash = %d\n", i);

     p = tbl->lists[i];

     list_for_each_entry_safe(entry, tmp, &p->list, list)
     {
        printk("key = %s, value = %s\n",entry->key, entry->value);
        if(strcmp(key, entry->key) == 0)
        {
            kfree(entry->key);
            kfree(entry->value);
            list_del_init(&entry->list);
        }
    }
}


void h_destroy(hashtbl *hash)
{
    int i = 0;
    hashlist *p;
    hashlist *ele, *tmp;
    for(i = 0; i < hash->tsize;i++)
    {
        p = hash->lists[i];
        list_for_each_entry_safe(ele, tmp, &p->list, list)
        {
            printk("key =%s value = %s\n", ele->key, ele->value);
            kfree(ele->key);
            kfree(ele->value);
            list_del_init(&ele->list);

        }
        
    }
    hash->tsize = 0;
}


unsigned int DJB_hash(const char *str)    ;
static int device_open(struct inode*, struct file*);
static ssize_t device_read(struct file*, char *, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);
static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

static struct file_operations fops =
{
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .unlocked_ioctl = device_ioctl,
};

static int device_open(struct inode* inode, struct file* file)
{
        return 0;
}
static ssize_t device_read(struct file* filp, char *buffer, size_t length, loff_t *offset)
{
        return 0;
}
static ssize_t device_write(struct file* filp, const char *buff, size_t len, loff_t *off)
{
        return 0;
}

static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
     int ret = 0;
     unsigned int hash = 0;
     char *p = NULL;
     char *q = NULL;
     char key[128] = {'\0'};
     char value[128] = {'\0'};
     printk("%s trying (pid=%d, comm=%s)\n", __func__, current->pid, current->comm);
     down(&sem);
     switch(cmd)
          {
               case IOCTL_GET:
                    memset(msg, '\0', 128);
                    ret = copy_from_user(msg, (unsigned char *)arg, 127);
                    find(msg, value, tbl);

                    ret = copy_to_user((unsigned char *)arg, value, strlen(value));
                                        

                    if(ret != 0)
                    {
                        ret = -EFAULT;
                    }
                    break;
               case IOCTL_SET:
                    memset(msg, '\0', 128);
                    ret = copy_from_user(msg, (unsigned char *)arg, 127);
                   // hash = DJB_hash(msg);
                    q = msg;
                    p = strstr(q, ":");
                    memcpy(key, q, p - q);
                    sprintf(value, "%s", &p[1]);
                    insert(key, value, tbl);
                    printk("<------CMD MEMDEV_IOCTL_SET Done------ %s---%s %u>\n\n", key, value, hash);
                    break;
               default:
                    ret = -EINVAL;
          }
     up(&sem);
     return ret;
}

static int hello_init(void)  
{ 
    sema_init(&sem, 1); 
    tbl = inithash(8);
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if(major < 0)
    {
        return major;
    }

    firstdrv_class = class_create(THIS_MODULE, "firstdrv");  
          
    firstdrv_device = device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME); 

    printk(KERN_ALERT "hash table init /n");  
    return 0;  
}  
static void hello_exit(void)  
{  
    h_destroy(tbl);
    unregister_chrdev(major, DEVICE_NAME);
    device_unregister(firstdrv_device);  //卸载类下的设备  
    class_destroy(firstdrv_class);      //卸载类  

    printk(KERN_ALERT "hash table exit /n");  
}  
module_init(hello_init);  
module_exit(hello_exit); 
