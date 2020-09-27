#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <asm/ioctl.h>
#include <linux/jiffies.h>
#include <linux/time.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "module2"
//This is the constant that used for determination of buffer length
#define MAX_BUF_LEN 4096


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Amir Sabzi");
MODULE_DESCRIPTION("this module should be create two proc interface");
MODULE_VERSION("1.0.0");


//Creating a proc directory entry structure
static struct proc_dir_entry* dir;

static struct proc_dir_entry* our_proc1_file;
static struct proc_dir_entry* our_proc2_file;
//Now we have to create a buffer for our longest message (MAX_BUF_LEN)
//and a variable to count our actual message size
static char procfs1_buffer[MAX_BUF_LEN],procfs2_buffer[MAX_BUF_LEN];
static unsigned long procfs1_buffer_size = 0;
static unsigned long procfs2_buffer_size = 0;



int procfs_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "module2: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	try_module_get(THIS_MODULE);
	return SUCCESS;
}


static ssize_t procfs1_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	static int ret = 0;
	printk(KERN_INFO "module2: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if(ret){
		printk(KERN_INFO "module2: Read END\n");
		ret = 0;
	}
	else{
		if(raw_copy_to_user(buffer, procfs1_buffer, procfs1_buffer_size))
			return -EFAULT;
		ret = procfs1_buffer_size;
	}

	return ret;
}
static ssize_t procfs2_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	static int ret = 0;
	printk(KERN_INFO "module2: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if(ret){
		printk(KERN_INFO "module2: Read END\n");
		ret = 0;
	}
	else{
		if(raw_copy_to_user(buffer, procfs2_buffer, procfs2_buffer_size))
			return -EFAULT;
		ret = procfs2_buffer_size;
	}
	return ret;
}


static ssize_t procfs1_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "module2: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if(length > MAX_BUF_LEN)
		procfs2_buffer_size = MAX_BUF_LEN;
	else
		procfs2_buffer_size = length;
	if(raw_copy_from_user(procfs2_buffer, buffer, procfs2_buffer_size))
		return -EFAULT;

	return procfs2_buffer_size;
}
static ssize_t procfs2_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "module2: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if(length > MAX_BUF_LEN)
		procfs1_buffer_size = MAX_BUF_LEN;
	else
		procfs1_buffer_size = length;
	if(raw_copy_from_user(procfs1_buffer, buffer, procfs1_buffer_size))
		return -EFAULT;
	return procfs1_buffer_size;
}


int procfs_close(struct inode *inode, struct file *file){
	printk(KERN_INFO "module2: Release Function, Process \"%s:%i\"\n", current->comm, current->pid);
	module_put(THIS_MODULE);
	return SUCCESS;
}



static const struct file_operations fops1 = {
	.owner = THIS_MODULE,
	.read = procfs1_read,
	.write = procfs1_write,
	.open = procfs_open,
	.release = procfs_close,
};

static const struct file_operations fops2 = {
	.owner = THIS_MODULE,
	.read = procfs2_read,
	.write = procfs2_write,
	.open = procfs_open,
	.release = procfs_close,
};


//Your module's entry point
static int __init readwrite_procfs_init(void){
	dir = proc_mkdir("proc_folder",NULL);
	printk(KERN_INFO "module2: Initialization.\n");
	printk(KERN_INFO "module2: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	our_proc1_file = proc_create("proc1", 0644 , dir, &fops1);
	if(!our_proc1_file){
		printk(KERN_ALERT "module2: Registration Failure.\n");
		return -ENOMEM;
	}

	our_proc2_file = proc_create("proc2", 0644 , dir, &fops2);
	if(!our_proc2_file){
		printk(KERN_ALERT "module2: Registration Failure.\n");
		return -ENOMEM;
	}

	printk(KERN_INFO "module2: /proc2/%s has been created.\n", DEVICE_NAME);
	return SUCCESS;
}


static void __exit readwrite_procfs_exit(void){
	printk(KERN_INFO "module2: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);
	remove_proc_entry("proc1", NULL);
	remove_proc_entry("proc2", NULL);
	printk(KERN_INFO "module2: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "module2: GoodBye.\n");
}

module_init(readwrite_procfs_init);
module_exit(readwrite_procfs_exit);
