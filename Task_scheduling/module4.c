//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For jiffies, which will give us timelapce of the system
#include <linux/jiffies.h>
//For catching CPU number
#include <linux/smp.h>
//Ofcourse for using workqueues
#include <linux/workqueue.h>
//For using Tasklets
#include <linux/interrupt.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "module4"

//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Amir Sabzi <97.amirsabzi@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module to compare taklet, workqueue and shared queue delay");
MODULE_VERSION("1.0.0");


//Define a workqueue
static struct workqueue_struct *our_workqueue1;
static struct workqueue_struct *our_workqueue2;
static struct workqueue_struct *our_workqueue3;
static struct workqueue_struct *our_workqueue4;

//Now we will define our tasklet_struct variable for future use
static struct tasklet_struct our_tasklet1;
static struct tasklet_struct our_tasklet2;
static struct tasklet_struct our_tasklet3;
static struct tasklet_struct our_tasklet4;

//Here are some useful variables
static unsigned long before_delay1, after_delay1,before_delay2, after_delay2;
static unsigned long before_delay3, after_delay3,before_delay4, after_delay4;
static unsigned long before_delay5, after_delay5,before_delay6, after_delay6;
static char our_tasklet_argument1[20] = DEVICE_NAME, string_argument1[20];
static char our_tasklet_argument2[20] = DEVICE_NAME, string_argument2[20];
static char our_tasklet_argument3[20] = DEVICE_NAME, string_argument3[20];
static char our_tasklet_argument4[20] = DEVICE_NAME, string_argument4[20];




static void our_tasklet_function1(unsigned long data){
	tasklet_trylock(&our_tasklet1);
	strcpy(string_argument1, data);
	after_delay1 = jiffies;
	printk(KERN_INFO "module4: Tasklet function of %s is running on CPU %d \n", string_argument1,smp_processor_id());
	printk(KERN_INFO "module4: Tasklet in workqueue function scheduled on: %ld \n", before_delay1);
	printk(KERN_INFO "module4: Tasklet in workqueue function executed on: %ld \n", after_delay1);
	printk(KERN_ALERT "module4: Tasklet in workqueue RUNTIME = %ld \n", after_delay1 - before_delay1);
	tasklet_unlock(&our_tasklet1);
}

static void our_work_function1(struct work_struct *our_work){
	printk(KERN_INFO "module4: Work function is running on CPU %d \n", smp_processor_id());
	tasklet_init(&our_tasklet1, &our_tasklet_function1, (unsigned long) &our_tasklet_argument1);
	printk(KERN_INFO "module4: Tasklet initiated on CPU %d \n", smp_processor_id());
	tasklet_schedule(&our_tasklet1);
}

static void our_work_function2(struct work_struct *our_work){
	printk(KERN_INFO "module4: Work function is running on CPU %d \n", smp_processor_id());
	after_delay2 = jiffies;
	printk(KERN_INFO "module4: Work function is running on CPU %d \n", smp_processor_id());
	printk(KERN_INFO "module4: workqueue in tasklet scheduled on: %ld \n", before_delay2);
	printk(KERN_INFO "module4: workqueue in tasklet executed on: %ld \n", after_delay2);
	printk(KERN_ALERT "module4: workqueue in tasklet RUNTIME = %ld \n", after_delay2 - before_delay2);
}


static DECLARE_DELAYED_WORK(our_work2, our_work_function2);

static void our_tasklet_function2(unsigned long data){
	tasklet_trylock(&our_tasklet2);
	strcpy(string_argument2, data);
	our_workqueue2 = create_singlethread_workqueue("ourqueue2");
	if(!our_workqueue2){
		printk(KERN_ALERT "module4: Creating the workqueue has been failed!\n");
	}
	queue_delayed_work(our_workqueue2, &our_work2, HZ);
	tasklet_unlock(&our_tasklet2);
}
static void our_work_function4(struct work_struct *our_work){
	after_delay3 = jiffies;
	printk(KERN_INFO "module4: Work function4 is running on CPU %d \n", smp_processor_id());
	printk(KERN_INFO "module4: sharedqueue in workqueue scheduled on: %ld \n", before_delay3);
	printk(KERN_INFO "module4: sharedqueue in workqueue executed on: %ld \n", after_delay3);
	printk(KERN_ALERT "module4: sharedqueue in workqueue RUNTIME = %ld \n", after_delay3 - before_delay3);

}
static DECLARE_DELAYED_WORK(our_work4, our_work_function4);

static void our_work_function3(struct work_struct *our_work){
	printk(KERN_INFO "module4: Work function3 is running on CPU %d \n", smp_processor_id());
	schedule_delayed_work(&our_work4,HZ);
	printk(KERN_INFO "sharedqueue1 has been created\n");
}

static void our_work_function6(struct work_struct *our_work){
	after_delay4 = jiffies;
	printk(KERN_INFO "module4: Work function6 is running on CPU %d \n", smp_processor_id());
	printk(KERN_INFO "module4: workqueue in sharedqueue scheduled on: %ld \n", before_delay4);
	printk(KERN_INFO "module4: workqueue in sharedqueue executed on: %ld \n", after_delay4);
	printk(KERN_ALERT "module4: workqueue in sharedqueue RUNTIME = %ld \n", after_delay4 - before_delay4);

}

static DECLARE_DELAYED_WORK(our_work6, our_work_function6);

static void our_work_function5(struct work_struct *our_work){
	printk(KERN_INFO "module4: Work function5 is running on CPU %d \n", smp_processor_id());
	our_workqueue4 = create_singlethread_workqueue("ourqueue4");
	if(!our_workqueue2){
		printk(KERN_ALERT "module4: Creating the workqueue4 has been failed!\n");
	}
	queue_delayed_work(our_workqueue4, &our_work6, HZ);
}

static void our_work_function7(struct work_struct *our_work){
	after_delay5 = jiffies;
	printk(KERN_INFO "module4: Work function7 is running on CPU %d \n", smp_processor_id());
	printk(KERN_INFO "module4: sharedqueue in tasklet scheduled on: %ld \n", before_delay5);
	printk(KERN_INFO "module4: sharedqueue in tasklet executed on: %ld \n", after_delay5);
	printk(KERN_ALERT "module4: sharedqueue in tasklet RUNTIME = %ld \n", after_delay5 - before_delay5);

}

static void our_tasklet_function4(unsigned long data){
	tasklet_trylock(&our_tasklet4);
	strcpy(string_argument4, data);
	after_delay6 = jiffies;
	printk(KERN_INFO "module4: tasklet in sharedqueue scheduled on: %ld \n", before_delay6);
	printk(KERN_INFO "module4: tasklet in sharedqueue executed on: %ld \n", after_delay6);
	printk(KERN_ALERT "module4: tasklet in sharedqueue RUNTIME = %ld \n", after_delay6 - before_delay6);
	tasklet_unlock(&our_tasklet4);
}

static void our_work_function8(struct work_struct *our_work){
	printk(KERN_INFO "module4: Work function8 is running on CPU %d \n", smp_processor_id());
	tasklet_init(&our_tasklet4, &our_tasklet_function4, (unsigned long) &our_tasklet_argument4);
	printk(KERN_INFO "module4: Tasklet4 initiated on CPU %d \n", smp_processor_id());
	tasklet_schedule(&our_tasklet4);

}

static DECLARE_DELAYED_WORK(our_work1, our_work_function1);
static DECLARE_DELAYED_WORK(our_work3, our_work_function3);
static DECLARE_DELAYED_WORK(our_work5, our_work_function5);
static DECLARE_DELAYED_WORK(our_work7, our_work_function7);
static DECLARE_DELAYED_WORK(our_work8, our_work_function8);

static void our_tasklet_function3(unsigned long data){
	tasklet_trylock(&our_tasklet3);
	strcpy(string_argument3, data);
	printk(KERN_INFO "module4: our_tasklet_function3 is running on CPU %d \n", smp_processor_id());
	schedule_delayed_work(&our_work7,HZ);
	printk(KERN_INFO "sharedqueue3 has been created\n");
	tasklet_unlock(&our_tasklet3);
}

static int __init module4_init(void){
	printk(KERN_INFO "module4: Initialization.\n");

////////////////////////// 		Tasklet in workqueue	////////////////////////////
	//Then we have to initiate a work
	before_delay1 = jiffies;
	our_workqueue1 = create_singlethread_workqueue("ourqueue1");
	if(!our_workqueue1){
		printk(KERN_ALERT "module4: Creating the workqueue has been failed!\n");
		return -EFAULT;
	}
	printk(KERN_INFO "module4: Workqueue1 has been created\n");
	queue_delayed_work(our_workqueue1, &our_work1, HZ);
	printk(KERN_INFO "module4: our work function and workqueue initiated on CPU %d \n", smp_processor_id());

////////////////////////// 		workqueue in tasklet	////////////////////////////
	before_delay2 = jiffies;
	tasklet_init(&our_tasklet2, &our_tasklet_function2, (unsigned long) &our_tasklet_argument2);
	printk(KERN_INFO "module4: Tasklet2 initiated on CPU %d \n", smp_processor_id());
	tasklet_schedule(&our_tasklet2);

//////////////////////////	SharedQueue in workqueue ///////////////////////////
	before_delay3 = jiffies;
	our_workqueue3 = create_singlethread_workqueue("ourqueue3");
	if(!our_workqueue3){
		printk(KERN_ALERT "module4: Creating the workqueue3 has been failed!\n");
		return -EFAULT;
	}
	printk(KERN_INFO "module4: Workqueue3 has been created\n");
	queue_delayed_work(our_workqueue3, &our_work3, HZ);

/////////////////////////  workqueue in sharedqueue ///////////////////////////
 	before_delay4 = jiffies;
 	schedule_delayed_work(&our_work5,HZ);
 	printk(KERN_INFO "sharedqueue2 has been created\n");

////////////////////////	sharedqueue in Tasklet   ////////////////////////////
	before_delay5 = jiffies;
	tasklet_init(&our_tasklet3, &our_tasklet_function3, (unsigned long) &our_tasklet_argument3);
	printk(KERN_INFO "module4: Tasklet3 initiated on CPU %d \n", smp_processor_id());
	tasklet_schedule(&our_tasklet3);

//////////////////////// tasklet in sharedqueue  //////////////////////////////
	before_delay6 = jiffies;
	schedule_delayed_work(&our_work8,HZ);
 	printk(KERN_INFO "sharedqueue2 has been created\n");







	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit module4_exit(void){
	//Now we have to get rid of our workqueue, so first we cancel the work,
	cancel_delayed_work(&our_work1);
	cancel_delayed_work(&our_work2);
	cancel_delayed_work(&our_work3);
	cancel_delayed_work(&our_work4);
	cancel_delayed_work(&our_work5);
	cancel_delayed_work(&our_work6);
	cancel_delayed_work(&our_work7);
	cancel_delayed_work(&our_work8);
	if(our_workqueue1)
		destroy_workqueue(our_workqueue1);
	if(our_workqueue2)
		destroy_workqueue(our_workqueue2);
	if(our_workqueue3)
		destroy_workqueue(our_workqueue3);
	if(our_workqueue4)
		destroy_workqueue(our_workqueue4);

	tasklet_disable_nosync(&our_tasklet1);
	tasklet_kill(&our_tasklet1);
	tasklet_disable_nosync(&our_tasklet2);
	tasklet_kill(&our_tasklet2);
	tasklet_disable_nosync(&our_tasklet3);
	tasklet_kill(&our_tasklet3);
	tasklet_disable_nosync(&our_tasklet4);
	tasklet_kill(&our_tasklet4);
	printk(KERN_INFO "module4: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(module4_init);
module_exit(module4_exit);
