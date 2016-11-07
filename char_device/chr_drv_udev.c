#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <asm/current.h>
#include <linux/device.h>

#define CHAR_DEV_NAME "rsa_mem_cdev"
#define MAX_LENGTH 4000
#define SUCCESS 0

static char *char_device_buf;
struct cdev *rsa_mem_cdev;
dev_t mydev;
int count=1;
static struct class *rsa_mem_class;

static int char_dev_open(struct inode *inode,
			    struct file  *file)
{
	static int counter = 0;
	counter++;
	printk(KERN_INFO "Number of times open() was called: %d\n", counter);
	printk (KERN_INFO " MAJOR number = %d, MINOR number = %d\n",imajor (inode), iminor (inode));
	printk(KERN_INFO "Process id of the current process: %d\n",current->pid );
	printk (KERN_INFO "ref=%lu\n", module_refcount(THIS_MODULE));
	return SUCCESS;
}

static int char_dev_release(struct inode *inode,
		            struct file *file)
{
	return SUCCESS;
}

static ssize_t char_dev_read(struct file *file,
		            char *buf,
			    size_t lbuf,
			    loff_t *ppos)
{
	int maxbytes; /* number of bytes from ppos to MAX_LENGTH */
	int bytes_to_do; /* number of bytes to read */
	int nbytes; /* number of bytes actually read */

	maxbytes = MAX_LENGTH - *ppos;

	if( maxbytes > lbuf ) bytes_to_do = lbuf;
	else bytes_to_do = maxbytes;

	if( bytes_to_do == 0 ) {
		printk("Reached end of device\n");
		return -ENOSPC; /* Causes read() to return EOF */
	}

	nbytes = bytes_to_do -
		 copy_to_user( buf, /* to */
			       char_device_buf + *ppos, /* from */
			       bytes_to_do ); /* how many bytes */

	*ppos += nbytes;
	return nbytes;
}

static ssize_t char_dev_write(struct file *file,
		            const char *buf,
			    size_t lbuf,
			    loff_t *ppos)
{
	int nbytes; /* Number of bytes written */
	int bytes_to_do; /* Number of bytes to write */
	int maxbytes; /* Maximum number of bytes that can be written */
	int i;

	maxbytes = MAX_LENGTH - *ppos;

	if( maxbytes > lbuf ) bytes_to_do = lbuf;
	else bytes_to_do = maxbytes;

	if( bytes_to_do == 0 ) {
		printk("Reached end of device\n");
		return -ENOSPC; /* Returns EOF at write() */
	}

	nbytes = bytes_to_do -
	         copy_from_user( char_device_buf + *ppos, /* to */
				 buf, /* from */
				 bytes_to_do ); /* how many bytes */
	for ( i = *ppos; i <= nbytes; i++)
	    if ( char_device_buf[i] != '&' && ( i >= 300 && i <400 ) )
		printk(KERN_INFO "Kernel Module char_device_buf %d overwritten\n", i);
	*ppos += nbytes;
	return nbytes;
}


static loff_t char_dev_lseek (struct file *file, loff_t offset, int orig)
{
    loff_t testpos;
    switch (orig) {

    case 0:                 /* SEEK_SET */
            testpos = offset;
            break;
    case 1:                 /* SEEK_CUR */
            testpos = file->f_pos + offset;
            break;
    case 2:                 /* SEEK_END */
           testpos = MAX_LENGTH + offset;
           break;
    default:
          return -EINVAL;
    }

    testpos = testpos < MAX_LENGTH ? testpos : MAX_LENGTH;
    testpos = testpos >= 0 ? testpos : 0;
    file->f_pos = testpos;
    printk (KERN_INFO "Seeking to pos=%ld\n", (long)testpos);
    return testpos;
}


static struct file_operations char_dev_fops = {
	.owner = THIS_MODULE,
	.read = char_dev_read,
	.write = char_dev_write,
	.open = char_dev_open,
	.release = char_dev_release,
	.llseek = char_dev_lseek
};

static __init int char_dev_init(void)
{
	int ret;
	int i;

	if (alloc_chrdev_region (&mydev, 0, count, CHAR_DEV_NAME) < 0) {
            printk (KERN_ERR "failed to reserve major/minor range\n");
            return -1;
    }

        if (!(rsa_mem_cdev = cdev_alloc ())) {
            printk (KERN_ERR "cdev_alloc() failed\n");
            unregister_chrdev_region (mydev, count);
            return -1;
 	}
	cdev_init(rsa_mem_cdev,&char_dev_fops);

	ret=cdev_add(rsa_mem_cdev,mydev,count);
	if( ret < 0 ) {
		printk(KERN_INFO "Error registering device driver\n");
	        cdev_del (rsa_mem_cdev);
                unregister_chrdev_region (mydev, count);
		return -1;
	}

	rsa_mem_class = class_create (THIS_MODULE, "VIRTUAL");
        device_create (rsa_mem_class, NULL, mydev, NULL, "%s", "rsa_mem_cdev");

	printk(KERN_INFO "\nDevice Registered: %s\n",CHAR_DEV_NAME);
	printk (KERN_INFO "Major number = %d, Minor number = %d\n", MAJOR (mydev),MINOR (mydev));

	char_device_buf =(char *)kmalloc(MAX_LENGTH,GFP_KERNEL);
	for (i = 300; i < 400; i++)
	{
	    	char_device_buf[i] = '&';
		/* Don't need to print the canary */
		printk(KERN_INFO "The character device buffer %d has: %c\n", i, char_device_buf[i]);
	}

	printk(KERN_INFO "Char device address range: %p - %p\n", char_device_buf, (char_device_buf+MAX_LENGTH));
	printk(KERN_INFO "Char device PHYS address range: %llu - %llu\n", virt_to_phys(char_device_buf), virt_to_phys((char_device_buf+MAX_LENGTH)));

	return 0;
}

static __exit void  char_dev_exit(void)
{
	 device_destroy (rsa_mem_class, mydev);
         class_destroy (rsa_mem_class);
	 cdev_del(rsa_mem_cdev);
	 unregister_chrdev_region(mydev,1);
	 kfree(char_device_buf);
	 printk(KERN_INFO "\n Driver unregistered \n");
}
module_init(char_dev_init);
module_exit(char_dev_exit);
MODULE_AUTHOR("Neel Shah");
MODULE_DESCRIPTION("RSA Memory Interface Device");
MODULE_LICENSE("GPL");
