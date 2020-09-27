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




//define module name
#define DEVICE_NAME "module1"
//define some macros for readability
#define SUCCESS 0
#define  DEV_BUF_LEN 4096

//define buffer and pointer to it
static char dev_buffer[DEV_BUF_LEN];
static unsigned long dev_buffer_size = 0;
//define a number to simulating mutexing
static int  Device_Open = 0;
//define major number
static int major;
//define log
static char  log[DEV_BUF_LEN] ;
//sprintf(log , "********log File******\n");

//define some useless stuff
static int gmt_hour = 3;
static int gmt_minute = 30;
static char gmt_sign = '+';
//static int counter = 0;
//Creating a proc directory entry structure
static struct proc_dir_entry* proc_file;



//define module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amir Sabzi");
MODULE_DESCRIPTION("this module should be create a dev interface and and a proc interface");
MODULE_VERSION("1.0.0");

static void write_log(char * local_log,size_t local_log_size){

	sprintf( log + strlen(log) ,local_log);
	struct timespec my_timeofday_gmt = current_kernel_time();
	struct timespec my_timeofday_loc;
	getnstimeofday(&my_timeofday_loc);
	int hour_gmt, minute_gmt, second_gmt;
	int hour_loc, minute_loc, second_loc;
	second_gmt = (int) my_timeofday_gmt.tv_sec;
	hour_gmt = (second_gmt / 3600) % 24;
	minute_gmt = (second_gmt / 60) % 60;
	second_gmt %= 60;
	second_loc = (int) my_timeofday_loc.tv_sec;
	hour_loc = ((second_loc / 3600) % 24 + gmt_hour) % 24;
	minute_loc = (second_loc / 60) % 60 + gmt_minute;

	if(minute_loc>=60){
		hour_loc = (hour_loc + 1) % 24;
		minute_loc %= 60;
		}
	second_loc %= 60;
	sprintf(log + strlen(log) , "GMT %d:%d:%d\tLocal Time %d:%d:%d (GMT %c%d:%d)\n",hour_gmt, minute_gmt, second_gmt, hour_loc, minute_loc, second_loc, gmt_sign, gmt_hour, gmt_minute);

}

static int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "module1: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if (Device_Open)
		return -EBUSY;
	Device_Open++;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
  static int ret = 0;
  	printk(KERN_INFO "module1: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);
  	if(ret){
  		printk(KERN_INFO "module1: /dev entry read has END\n");
  		ret = 0;
  	}
  	else{
  		if(raw_copy_to_user(buffer, dev_buffer, dev_buffer_size))
  			return -EFAULT;
  		printk(KERN_INFO "module1: %lu bytes has read from /dev entry\n", dev_buffer_size);
  		ret = dev_buffer_size;
  	}
  	return ret;
}




static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "module1: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if (length > DEV_BUF_LEN)
		dev_buffer_size = DEV_BUF_LEN;
	else
		dev_buffer_size = length;

	if(raw_copy_from_user(dev_buffer, buffer, dev_buffer_size))
		return -EFAULT;

	write_log(dev_buffer,length);

	return dev_buffer_size;
}

int device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "module1: Release Function, Process \"%s:%i\"\n", current->comm, current->pid);
	Device_Open--;
	module_put(THIS_MODULE);
	return SUCCESS;
}

///////////////////////////////proc functions /////////////////////////////////
static int proc_show(struct seq_file *m, void *v){
	printk(KERN_EMERG "module1: you entered the show function and next lines are log\n");
	seq_printf(m, "%s", log);
	return SUCCESS;
}

static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "module1: Proc Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show, NULL);
}




//define file_operations we need in this module
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

static const struct file_operations pops = {
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

//init module
static int __init hw1_module_init(void){
  printk(KERN_INFO "module1 module initialize successfully\n");
  printk(KERN_INFO "module1: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

  ///////////create proc interface////////////////////
  proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &pops);
  if(!proc_file){
		printk(KERN_ALERT "module1: Registration Failure.\n");
		return -ENOMEM;
	}
	printk(KERN_INFO "module1: /proc/%s has been created.\n", DEVICE_NAME);
  /////////////create dev interface///////////////////////
  major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ALERT "module1: Registration Filure, %d Major\n", major);
		return major;
		}
	printk(KERN_INFO "module1: 'mknod /dev/%s c %d 0'\n", DEVICE_NAME, major);
	printk(KERN_INFO "module1: 'chmod 777 /dev/%s'.\n", DEVICE_NAME);

  return SUCCESS ;

}

static void __exit hw1_module_exit(void){
	printk(KERN_INFO "module1: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "module1: /proc/%s has been removed.\n", DEVICE_NAME);

	unregister_chrdev(major, DEVICE_NAME);
	printk(KERN_INFO "module1: 'rm /dev/%s'\n", DEVICE_NAME);

	printk(KERN_INFO "module1: GoodBye.\n");
}
module_init(hw1_module_init);
module_exit(hw1_module_exit);
