#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/utsname.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <asm/ioctl.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include "ioctl_commands.h"
//It is always good to have a meaningful constant as a return code
//This will be our module name
#define DEVICE_NAME "module3"
//This is the constant that used for determination of buffer length
#define standard_length 128
#define MAX_BUF_LEN 4096
#define SUCCESS 0
//descriptions
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amir Sabzi");
MODULE_DESCRIPTION("this module just a test module for assignment 2 of OS course");
MODULE_VERSION("1.0.0");
/////////////////////////dev interface variables////////////////////////////////
static char dev_sstack_buffer[MAX_BUF_LEN];
static int dev_sstack_ptr = 0;
static char dev_sfifo_buffer[MAX_BUF_LEN];
static int dev_sfifo_first = 0;
static int dev_sfifo_last =0;
static int fifo_counter = 0;
static int stack_counter =0;
static int  Device_Open = 0;
static int major;
/////////////////////////sys interface variables////////////////////////////////
static struct kobject *our_kobj;
static char sys_sfifo_buffer[MAX_BUF_LEN] ="0\n";
static char sys_sstack_buffer[MAX_BUF_LEN]="0\n";
/////////////////////////proc interface variables////////////////////////////////
static struct proc_dir_entry* plog_file;
static struct proc_dir_entry* pstack_file;
static struct proc_dir_entry* pfifo_file;
static char plog_buffer[MAX_BUF_LEN],pstack_buffer[MAX_BUF_LEN],pfifo_buffer[MAX_BUF_LEN];
//////////////////////start of ioctl interface functions////////////////////////
static long proc_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
  //static int err = 0;
  printk(KERN_INFO "module3: IOCTL Function, Process \"%s:%i\"\n", current->comm, current->pid);
	printk(KERN_INFO "module3: IOCTL Command, %d\n", cmd);
  if(capable(CAP_NET_RAW))
		printk(KERN_INFO "module3: Caller is Capable\n");
	else
	{
		printk(KERN_INFO "module3: Caller is not Capable\n");
		return -EPERM;
	}
  if(_IOC_TYPE(cmd) != MAGIC || _IOC_NR(cmd) > IOC_MAXNR)
		return -ENOTTY;
  switch(cmd){
		case IOCTL_RESET_FIFO:
			dev_sfifo_first = 0;
      dev_sfifo_last  = 0;
      fifo_counter = 0;
			break;
		case IOCTL_RESET_STACK:
			dev_sstack_ptr = 0;
      stack_counter = 0;
			break;
		case IOCTL_RESET_ALL:
      dev_sfifo_first = 0;
      dev_sfifo_last  = 0;
      dev_sstack_ptr = 0;
      stack_counter = 0;
      fifo_counter = 0;
			break;
    default:
			printk(KERN_ALERT "module3: Invalid IOCTL Command!\n");
			return -ENOTTY;
    }
    return SUCCESS;
}
//////////////////////start of dev interface functions////////////////////////
static int write_stack(char *input,int input_length){
  int i = 0;
  if(input_length > standard_length){
    raw_copy_from_user(dev_sstack_buffer + dev_sstack_ptr , input , standard_length );
  }else{
    raw_copy_from_user(dev_sstack_buffer + dev_sstack_ptr , input , input_length );
    for (i=input_length-1;i<standard_length-1;i++)
      dev_sstack_buffer[i + dev_sstack_ptr]='\0';
    dev_sstack_buffer[dev_sstack_ptr + standard_length-1] = '\n';
  }
  dev_sstack_ptr += standard_length ;
  return SUCCESS;
}

static char* read_stack(char * output){
  if (dev_sstack_ptr > 0){
    raw_copy_to_user( output, dev_sstack_buffer + dev_sstack_ptr - standard_length , standard_length );
	  dev_sstack_ptr -= standard_length ;
    return SUCCESS ;
  }
  return SUCCESS+1;
}

static int write_fifo(char *input,int input_length){
  int i = 0;
  if(input_length > standard_length){
    raw_copy_from_user(dev_sfifo_buffer + dev_sfifo_last , input , standard_length );
  }else{
    raw_copy_from_user(dev_sfifo_buffer + dev_sfifo_last , input , input_length );
    for (i=input_length-1;i<standard_length-1;i++)
      dev_sfifo_buffer[i + dev_sfifo_last]='\0';
    dev_sfifo_buffer[dev_sfifo_last + standard_length-1] = '\n';
  }
  dev_sfifo_last += standard_length ;
  return SUCCESS;
}

static char* read_fifo(char * output){
  if(dev_sfifo_first < dev_sfifo_last){
     raw_copy_to_user(output,dev_sfifo_buffer + dev_sfifo_first,standard_length);
	   dev_sfifo_first += standard_length ;
     return SUCCESS;
  }

  return SUCCESS+1 ;
}


static int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "module3: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if (Device_Open)
		return -EBUSY;
	Device_Open++;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
  static int ret = 0;
  	printk(KERN_INFO "module3: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);
  	if(ret){
  		printk(KERN_INFO "module3: /dev entry read has END\n");
  		ret = 0;
  	}
  	else{
			if(strcmp(sys_sstack_buffer,"1\n") == 0 && strcmp(sys_sfifo_buffer,"1\n") != 0){ //stack
				if(read_stack(buffer))
					ret = 0;
        else{
          ret = standard_length;
          stack_counter --;
        }
			}else if(strcmp(sys_sfifo_buffer,"1\n") == 0 && strcmp(sys_sstack_buffer,"1\n") != 0){ //fifo
				if(read_fifo(buffer))
					   ret = 0;
        else{
          ret = standard_length;
          fifo_counter -- ;
        }

			}else{ //none
        printk(KERN_INFO "module3: you should turn on only one of the sstack or sfifo sys interface\n");
				ret = 0;
			}
  		printk(KERN_INFO "module3: bytes has read from /dev entry\n");
  	}
    return ret ;

}

static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "module3: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	int err = 0;
	if(strcmp(sys_sstack_buffer,"1\n") == 0 && strcmp(sys_sfifo_buffer,"1\n") == 0){			//sfifo and sstack

		err += write_stack(buffer,length);
		err += write_fifo(buffer,length);
		if(err)
			return -EFAULT;
		fifo_counter ++ ;
		stack_counter ++ ;
	}else if(strcmp(sys_sfifo_buffer,"1\n") == 0 && strcmp(sys_sstack_buffer,"1\n") != 0){   //sfifo
		err += write_fifo(buffer,length);
		if(err)
			return -EFAULT;
		fifo_counter ++;
	}else if(strcmp(sys_sfifo_buffer,"1\n") != 0 && strcmp(sys_sstack_buffer,"1\n") == 0){	//sstack
		err += write_stack(buffer,length);
		if(err)
			return -EFAULT;
		stack_counter ++;
	}else{
		printk(KERN_INFO "module3: you should turn on at least one of the sstack or sfifo sys interface\n");
    return -EFAULT;
	}
  return standard_length;
}
static int device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "module3: Release Function, Process \"%s:%i\"\n", current->comm, current->pid);
	Device_Open--;
	module_put(THIS_MODULE);
	return SUCCESS;
}
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};
/////////////////////////end of dev interface functions////////////////////////
/*****************************************************************************/
////////////////////////start of sys interface functions///////////////////////
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	printk(KERN_INFO "module3: Show Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);
	if(strcmp(attr->attr.name, "sfifo") == 0)
		return sprintf(buf, "%s", sys_sfifo_buffer);
	else if (strcmp(attr->attr.name, "sstack") == 0)
		return sprintf(buf, "%s", sys_sstack_buffer);
	else
		printk(KERN_INFO  "module3: I don't know what you are doing, but it seems you are not doing it right!\n");
	return NULL;
}
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	printk(KERN_INFO "module3:  Store Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);
	if(strcmp(attr->attr.name, "sfifo") == 0){
    	if(strcmp(buf,"1\n") == 0 || strcmp(buf,"0\n") == 0)
					sprintf(sys_sfifo_buffer,"%s",buf);
			else
					printk(KERN_ALERT "module3: only permitted values for sfifo attribute  is 0 and 1! and your value is %s\n",buf);
  }else if (strcmp(attr->attr.name, "sstack") == 0){
			if(strcmp(buf,"1\n") == 0 || strcmp(buf,"0\n") == 0)
					sprintf(sys_sstack_buffer,"%s",buf);
			else
					printk(KERN_ALERT "module3: only permitted values for sstack attribute  is 0 and 1! and your value is %s\n",buf);
	}else
		printk(KERN_ALERT "module3: I don't know what you are doing, but it seems you are not permitted!\n");
	return count;
}
static struct kobj_attribute sfifo_attribute = __ATTR(sfifo, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute sstack_attribute = __ATTR(sstack, 0664, sysfs_show, sysfs_store);
static struct attribute *attrs[] = {
	&sfifo_attribute.attr,
	&sstack_attribute.attr,
	NULL,
};
static struct attribute_group attr_group = {
	.attrs = attrs,
};
/////////////////////////end of sys interface functions////////////////////////
/*****************************************************************************/
////////////////////////start of proc interface functions///////////////////////
static int proc_plog_show(struct seq_file *m, void *v){
	printk(KERN_EMERG "module3: you entered the show function and next lines are log\n");
	seq_printf(m, "the number of stack items is: %d\n", stack_counter);
	seq_printf(m, "the number of fifo items is: %d\n", fifo_counter);
	return SUCCESS;
}
static int proc_plog_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "module3: Proc Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_plog_show, NULL);
}
static const struct file_operations log_ops = {
	.open = proc_plog_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
  .unlocked_ioctl = proc_ioctl,
};

static int proc_pfifo_show(struct seq_file *m, void *v){
	printk(KERN_EMERG "module3: you entered the show function and next lines are log\n");
  size_t i ;
  char temp[standard_length];
  for(i = 0 ;i < fifo_counter ; i++){
    strncpy(temp,dev_sfifo_buffer + dev_sfifo_first + i * standard_length ,standard_length);
    temp[standard_length-1] = '\0';
    seq_printf(m, "%s\n", temp);
  }
	return SUCCESS;
}
static int proc_pfifo_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "module3: Proc Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_pfifo_show, NULL);
}
static const struct file_operations fifo_ops = {
	.open = proc_pfifo_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int proc_pstack_show(struct seq_file *m, void *v){
	printk(KERN_EMERG "module3: you entered the show function and next lines are log\n");
  size_t i ;
  char temp[standard_length];
  for(i = 1 ;i <= stack_counter ; i++){
    strncpy(temp,dev_sstack_buffer + dev_sstack_ptr - standard_length*(i),standard_length);
    temp[standard_length-1] = '\0';
    seq_printf(m, "%s\n", temp);
  }
	return SUCCESS;
}
static int proc_pstack_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "module3: Proc Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_pstack_show, NULL);
}
static const struct file_operations stack_ops = {
	.open = proc_pstack_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};



static int __init hw3_module_init(void){
  printk(KERN_INFO "module3 module initialize successfully\n");
  printk(KERN_INFO "module3: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);
  //////////////////////create proc interface//////////////////////////////////
	printk(KERN_INFO "module3: /proc/%s has been created.\n", DEVICE_NAME);
  plog_file = proc_create("plog", 0644 , NULL, &log_ops);
  if(!plog_file){
		printk(KERN_ALERT "module3: pfifo Registration Failure.\n");
		return -ENOMEM;
	}
  pstack_file = proc_create("pstack",0644,NULL,&stack_ops);
  if(!pstack_file){
		printk(KERN_ALERT "module3: pfifo Registration Failure.\n");
		return -ENOMEM;
	}
  pfifo_file = proc_create("pfifo",0644,NULL,&fifo_ops);
  if(!pfifo_file){
    printk(KERN_ALERT "module3: pfifo Registration Failure.\n");
    return -ENOMEM;
  }
  ////////////////////////create dev interface/////////////////////////////////
  major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ALERT "module3: Registration Filure, %d Major\n", major);
		return major;
		}
	printk(KERN_INFO "module3: 'mknod /dev/%s c %d 0'\n", DEVICE_NAME, major);
	printk(KERN_INFO "module3: 'mknod /dev/%s c %d 0 && chmod 777 /dev/%s'.\n",DEVICE_NAME,major,DEVICE_NAME);
  ////////////////////////create sys interface//////////////////////////////////
  int return_value;
  our_kobj = kobject_create_and_add(DEVICE_NAME, NULL);
  if (!our_kobj){
    printk(KERN_ALERT "module3: KOBJECT Registration Failure.\n");
    return -ENOMEM;
  }
  return_value = sysfs_create_group(our_kobj, &attr_group);
  if (return_value){
    printk(KERN_ALERT "module3: Creating attribute groupe has been failed.\n");
    kobject_put(our_kobj);
  }

  return SUCCESS ;
}
static void __exit hw3_module_exit(void){
	printk(KERN_INFO "module3: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

  kobject_put(our_kobj);
	unregister_chrdev(major, DEVICE_NAME);
  remove_proc_entry("plog", NULL);
  remove_proc_entry("pstack", NULL);
  remove_proc_entry("pfifo", NULL);
	printk(KERN_INFO "module1: 'rm /dev/%s'\n", DEVICE_NAME);

	printk(KERN_INFO "module3: GoodBye.\n");
}
module_init(hw3_module_init);
module_exit(hw3_module_exit);
