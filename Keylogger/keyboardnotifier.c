//We are going to do some kernel programming
#include <linux/kernel.h>
//Obviously we are creating a module
#include <linux/module.h>
//Just for using macros here
#include <linux/init.h>
//For obtaining PID and process name
#include <linux/sched.h>
//For registering a keyboard notifier
#include <linux/keyboard.h>
//For handling concurrency issues
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <asm/current.h>


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Amir Sabzi");
MODULE_DESCRIPTION("This is just a simple module based on Linux Kernel notifier chains to monitor keyboard activities and pressed keys");
MODULE_VERSION("1.0.2");


struct semaphore sem;
#define SUCCESS 0
// in terms of VC code or Scan-Codes
// use linux showkey command to see the scancodes ...
/*
NAME
showkey - examine the codes sent by the keyboard
SYNOPSIS
showkey [-h|--help] [-a|--ascii] [-s|--scancodes] [-k|--keycodes] [-V|--version]
DESCRIPTION

showkey prints to standard output either the scan codes or the keycode or the `ascii' code of each key pressed. In the first two modes the program runs until 10 seconds have elapsed since the last key press or release event, or until it receives a suitable signal, like SIGTERM, from another process. In `ascii' mode the program terminates when the user types ^D.
*/

static const char* keys[] = { "\0", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "BACKSPACE", "TAB",
	"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "SPACE", "SPACE", "ENTER", "CTRL", "a", "s", "d", "f",
	"g", "h", "j", "k", "l", ";", "'", "`", "SHIFT", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".",
	"/", "SHIFT", "\0", "\0", "SPACE", "CAPSLOCK", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
	"F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK", "HOME", "UP", "PGUP", "-", "LEFT", "5",
	"RTARROW", "+", "END", "DOWN", "PGDN", "INS", "DELETE", "\0", "\0", "\0", "F11", "F12",
	"\0", "\0", "\0", "\0", "\0", "\0", "\0", "ENTER", "CTRL", "/", "PRTSCR", "ALT", "\0", "HOME",
	"UP", "PGUP", "LEFT", "RIGHT", "END", "DOWN", "PGDN", "INSERT", "DEL", "\0", "\0",
	"\0", "\0", "\0", "\0", "\0", "PAUSE"
};
static int keys_list_size = sizeof(keys)/8 ;
static int keys_num[sizeof(keys)/8]= {0};

static const char* shift_keys[] ={ "\0", "ESC", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "BACKSPACE", "TAB",
	"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "ENTER", "CTRL", "A", "S", "D", "F",
	"G", "H", "J", "K", "L", ":", "\"", "~", "SHIFT", "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">",
	"?", "SHIFT", "\0", "\0", "SPACE", "CAPSLOCK", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
	"F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK", "HOME", "UP", "PGUP", "-", "LEFT", "5",
	"RTARROW", "+", "END", "DOWN", "PGDN", "INS", "DELETE", "\0", "\0", "\0", "F11", "F12",
	"\0", "\0", "\0", "\0", "\0", "\0", "\0", "ENTER", "CTRL", "/", "PRTSCR", "ALT", "\0", "HOME",
	"UP", "PGUP", "LEFT", "RIGHT", "END", "DOWN", "PGDN", "INSERT", "DEL", "\0", "\0",
	"\0", "\0", "\0", "\0", "\0", "PAUSE"
	};
static int shift_keys_list_size = sizeof(shift_keys)/8;
static int shift_keys_num[sizeof(shift_keys)/8]= {0};

static int shift_key_flag = 0;
static int error = 0;
static int i ;
static struct proc_dir_entry* proc_file;


int keyboard_notify( struct notifier_block *nblock, unsigned long code, void *_param ){
	//Know which keys has triggered this callback function
	struct keyboard_notifier_param *param = _param;

	if( param->value==42 || param->value==54 ){
		down(&sem);
		if(param->down)
			shift_key_flag = 1;
		else
			shift_key_flag = 0;
		up(&sem);
		return NOTIFY_DONE;
	}
	if(code == KBD_KEYCODE){
		if(param->down){
			down(&sem);
			if(shift_key_flag == 0){
				keys_num[param->value]++ ;
				//printk(KERN_INFO "%s pressed %d times\n",keys[param->value],keys_num[param->value]);
			}else if(shift_key_flag == 1){
				shift_keys_num[param->value]++ ;
				//printk(KERN_INFO "%s pressed %d times\n",shift_keys[param->value],shift_keys_num[param->value]);
			}else
				printk(KERN_INFO "SIMPLEKEYBOARDNOFIFIER: Buffer Overflow\n");
			up(&sem);
			return NOTIFY_OK;
		}
	}
	return NOTIFY_OK;

	// if NOK return NOTIFY_STOP; means just eat the event!
}

static int proc_show(struct seq_file *m, void *v){
	seq_printf(m, "|\tkey\t\t|\tnum of pressing\t|\n");
	seq_printf(m, "--------------------------------------------------------------\n");

	for(i = 0; i < keys_list_size; i++) {
		if(keys_num[i])
			seq_printf(m, "|\t%s\t\t|\t       %d       \t\n",keys[i],keys_num[i]);
	}
	for (i = 0; i < shift_keys_list_size; i++) {
		if(shift_keys_num[i])
			seq_printf(m, "|\t%s\t\t|\t       %d       \t\n",shift_keys[i],shift_keys_num[i]);
	}
	seq_printf(m, "--------------------------------------------------------------\n	");
	return SUCCESS;
}
static int proc_open(struct inode *inode, struct file *file){
	return single_open(file, proc_show, NULL);
}
static const struct file_operations pops = {
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
//Using notifiers for identifying actions on loadable Kernel modules
static struct notifier_block keyboard_nb ={
	.notifier_call = keyboard_notify,  //This fuction will call whenever the notifier triggers
};


//This would be our module start up point
static int __init keyboard_notifier_init(void){
	printk(KERN_INFO "SIMPLEKEYBOARDNOFIFIER: Init module, Process(%s:%i)\n", current->comm, current->pid);
	//Registering a Notifier
	error = register_keyboard_notifier(&keyboard_nb);
	if(error){
		printk(KERN_ALERT "SIMPLEKEYBOARDNOFIFIER: Error in registering keyboard notifier\n");
		return error;
		}

	proc_file = proc_create("kb_history", 0644 , NULL, &pops);
	printk(KERN_INFO "SIMPLEKEYBOARDNOFIFIER: Keyboard notifier registered successfully\n");

	sema_init(&sem, 1);
	return error;

}


//Now, due to end this module, its time to clean up the mess
static void __exit keyboard_notifier_exit(void){
	printk(KERN_INFO "SIMPLEKEYBOARDNOFIFIER: Init module, Process(%s:%i)\n", current->comm, current->pid);
	//Unregistering keyboard notifier
	unregister_keyboard_notifier(&keyboard_nb);
	remove_proc_entry("kb_history", NULL);

        printk(KERN_INFO "SIMPLEKEYBOARDNOFIFIER: Keyboard notifier deregistered\n");
}


//Definition of init_module and cleanup_modules with user-defined functions
module_init(keyboard_notifier_init);
module_exit(keyboard_notifier_exit);
