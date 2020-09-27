#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/signal.h>
#undef debug


// ATTENTION copied from /uboot_for_mpc/arch/powerpc/include/asm/signal.h
// Maybe it don't work with that
//____________________________________________________________
#define SA_INTERRUPT    0x20000000 /* dummy -- ignored */
#define SA_SHIRQ        0x04000000
//____________________________________________________________

#define pci_module_init pci_register_driver // function is obsoleted

// Hardware specific part
#define MY_VENDOR_ID 0x5333
#define MY_DEVICE_ID 0x8e40


#define MAJOR_NR     25
#define DRIVER_NAME  "PCI-Driver"



static unsigned long ioport=0L, iolen=0L, memstart=0L, memlen=0L,flag0,flag1,flag2,temp=0L;

// private_data
struct _instance_data {

    int counter; // just as a example (5-27)

    // other instance specific data
};

// Interrupt Service Routine
static irqreturn_t pci_isr( int irq, void *dev_id)
{
    return IRQ_HANDLED;
}


// Check if this driver is for the new device
static int device_init(struct pci_dev *dev,
        const struct pci_device_id *id)
{
    int err=0;  // temp variable

    #ifdef debug

    flag0=pci_resource_flags(dev, 0 );
    flag1=pci_resource_flags(dev, 1 );
    flag2=pci_resource_flags(dev, 2 );
    printk("DEBUG: FLAGS0 = %u\n",flag0);
    printk("DEBUG: FLAGS1 = %u\n",flag1);
    printk("DEBUG: FLAGS2 = %u\n",flag2);

    /*
     * The following sequence checks if the resource is in the
     * IO / Storage / Interrupt / DMA address space
     * and prints the result in the dmesg log
     */
    if(pci_resource_flags(dev,0) & IORESOURCE_IO)
    {
        // Ressource is in the IO address space
        printk("DEBUG: IORESOURCE_IO\n");
    }
    else if (pci_resource_flags(dev,0) & IORESOURCE_MEM)
    {
        // Resource is in the Storage address space
        printk("DEBUG: IORESOURCE_MEM\n");
    }
    else if (pci_resource_flags(dev,0) & IORESOURCE_IRQ)
    {
        // Resource is in the IRQ address space
        printk("DEBUG: IORESOURCE_IRQ\n");
    }
    else if (pci_resource_flags(dev,0) & IORESOURCE_DMA)
    {
        // Resource is in the DMA address space
        printk("DEBUG: IORESOURCE_DMA\n");
    }
    else
    {
        printk("DEBUG: NOTHING\n");
    }

    #endif /* debug */

    // allocate memory_region
    memstart = pci_resource_start( dev, 0 );
    memlen = pci_resource_len( dev, 0 );
    if( request_mem_region( memstart, memlen, dev->dev.kobj.name )==NULL ) {
        printk(KERN_ERR "Memory address conflict for device \"%s\"\n",
                dev->dev.kobj.name);
        return -EIO;
    }
    // allocate a interrupt
    if(request_irq(dev->irq,pci_isr,SA_INTERRUPT|SA_SHIRQ,
            "pci_drv",dev)) {
        printk( KERN_ERR "pci_drv: IRQ %d not free.\n", dev->irq );
    }
    else
    {
        err=pci_enable_device( dev );
        if(err==0)      // enable device successful
        {
            return 0;
        }
        else        // enable device not successful
        {
            return err;
        }

    }
    // cleanup_mem
    release_mem_region( memstart, memlen );
    return -EIO;
}
// Function for deinitialization of the device
static void device_deinit( struct pci_dev *pdev )
{
    free_irq( pdev->irq, pdev );
    if( memstart )
        release_mem_region( memstart, memlen );
}

static struct file_operations pci_fops;

static struct pci_device_id pci_drv_tbl[] = {
	{ PCI_DEVICE(MY_VENDOR_ID, MY_DEVICE_ID), },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci_drv, pci_drv_tbl);


static int driver_open( struct inode *geraetedatei, struct file *instance )
{
    struct _instance_data *iptr;

    iptr = (struct _instance_data *)kmalloc(sizeof(struct _instance_data),
            GFP_KERNEL);
    if( iptr==0 ) {
        printk("not enough kernel mem\n");
        return -ENOMEM;
    }
    /* replace the following line with your instructions  */
    iptr->counter= strlen("Hello World\n")+1;    // just as a example (5-27)

    instance->private_data = (void *)iptr;
    return 0;
}

static void driver_close( struct file *instance )
{
    if( instance->private_data )
        kfree( instance->private_data );
}


static struct pci_driver pci_drv = {
	          .name= "pci_drv",
            .id_table= pci_drv_tbl,
            .probe= device_init,
            .remove= device_deinit,
};

static int __init pci_drv_init(void)
{    // register the driver by the OS
    printk("DEBUG: FLAGS0");
    if(register_chrdev(MAJOR_NR, DRIVER_NAME, &pci_fops)==0) {
        if(pci_module_init(&pci_drv) == 0 ) // register by the subsystem
            return 0;
        unregister_chrdev(MAJOR_NR,DRIVER_NAME); // unregister if no subsystem support
    }
    return -EIO;
}

static void __exit pci_drv_exit(void)
{
    pci_unregister_driver( &pci_drv );
    unregister_chrdev(MAJOR_NR,DRIVER_NAME);
}

module_init(pci_drv_init);
module_exit(pci_drv_exit);

MODULE_LICENSE("GPL");
