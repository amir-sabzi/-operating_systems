/////////////////////////IOCTL interface variables//////////////////////////////
#define MAGIC 'A'
#define IOC_MAXNR 3
#define IOCTL_RESET_FIFO _IO(MAGIC, 0)
#define IOCTL_RESET_STACK _IO(MAGIC, 1)
#define IOCTL_RESET_ALL _IO(MAGIC, 2)
