#include <linux/init.h>                                                                    
#include <linux/module.h>                                                                  
#include <linux/fs.h>                                                                      
#include <linux/cdev.h>                                                                    
#include <linux/uaccess.h>                                                                 
#include <linux/slab.h>                                                                    
                                                                                           
struct cdriver_data {                                                                      
        struct cdev cdev;                                                                  
        char data[1024];                                                                   
        int size ;                                                                         
};                                                                                         
                                                                                           
static struct cdriver_data *cdriver_device;                                                
static struct class* cdriver_class ;                                                       
                                                                                           
static int cdriver_open(struct inode *inode,struct file *file){                            
        file->private_data = cdriver_device ;                                              
        return 0 ;                                                                         
}                                                                                          
                                                                                           
static ssize_t cdriver_read(struct file *file, char __user *buffer_to_user, size_t count ,loff_t *offset ) {
        struct cdriver_data *cd_data = (struct cdriver_data*) file-> private_data ;                                                                     
        if(count >= cd_data->size - *offset)                                               
                count= cd_data->size - *offset ;                                           
                                                                                           
        if(copy_to_user(buffer_to_user,cd_data->data+*offset,count))                                     
                return -EIO ;                                                              
                                                                                           
        if(*offset >= cd_data->size)                                                       
                return 0 ;                                                                 
        *offset+=count ;                                                                   
        return count ;                                                                     
}                                                                                          
                                                                                           
static ssize_t cdriver_write(struct file *file,const char __user *buffer_from_user,size_t count, loff_t *offset) {
        struct cdriver_data *cd_data = (struct cdriver_data*) file-> private_data ;        
        if(count > 1024 )                                                                  
                count = 1024 ;                                                             
        cd_data->size+=count ;                                                             
        if(copy_from_user(cd_data->data + cd_data->size,buffer_from_user,count))                         
                return -EIO ;                                                              
        return count ;                                                                     
}                                                                                          
                                                                                           
static int cdriver_release(struct inode *ind,struct file *f){                              
        return 0 ;                                                                         
}                                                                                          
                                                                                           
static struct file_operations cdriver_fops = {                                             
        .owner = THIS_MODULE,                                                              
        .open = cdriver_open,                                                              
        .read = cdriver_read,                                                              
        .write = cdriver_write,                                                            
        .release = cdriver_release                                                         
};                                                                                         
                                                                                           
static dev_t device_num ;                                                                  
                                                                                           
                                                                                           
int chardev_init(void){                                                                    
        int ret ;                                                                          
                                                                                           
        cdriver_device =  kzalloc(sizeof(struct cdriver_data), GFP_KERNEL);                
        if(!cdriver_device){                                                               
                return -ENOMEM ;                                                           
        }                                                                                  
        //Register region of the char driver                                               
                                                                                           
        alloc_chrdev_region(&device_num , 0 , 1 , "salim_cdev");                           
        if(ret< 0)                                                                         
                return ret ;                                                               
                                                                                           
        //init char driver                                                                 
        cdev_init(&cdriver_device->cdev,&cdriver_fops);                                    
        cdriver_device->cdev.owner = THIS_MODULE ;                                                                         
                                                                                           
        //add char driver                                                                  
        ret = cdev_add(&cdriver_device->cdev,device_num,1);                                
        if(ret<0){                                                                         
                kfree(cdriver_device);                                                     
                goto  error_handle ;                                                       
        }                                                                                  
                                                                                           
        //create class                                                                     
        cdriver_class = class_create(THIS_MODULE,"salim_cdev") ;                           
        if(IS_ERR(cdriver_class)){                                                         
                kfree(cdriver_class);                                                      
                goto error_handle ;                                                        
        }                                                                                  
                                                                                           
        //create device                                                                    
        device_create(cdriver_class,NULL,device_num,NULL,"salim") ;                        
                                                                                           
error_handle:                                                                              
        unregister_chrdev_region (device_num,1);                                           
        return ret ;                                                                       
}                                                                                          
                                                                                           
void chardev_exit(void){                                                                   
        if(cdriver_device)                                                                 
                device_destroy(cdriver_class,device_num);                                  
        cdev_del(&cdriver_device->cdev);                                                   
        if(cdriver_class)                                                                  
                class_destroy(cdriver_class);                                              
        if(device_num >=0 )                                                                
                unregister_chrdev_region(device_num,1);                                    
        pr_info("bye driver \n");                                                          
}                                                                                          
                                                                                           
MODULE_LICENSE("GPL");                                                                     
MODULE_AUTHOR("salim");                                                                    
MODULE_DESCRIPTION("Character device driver example");                                     
module_init(chardev_init);                                                                 
module_exit(chardev_exit);
