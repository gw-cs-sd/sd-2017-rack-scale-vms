
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

int main()
{
	int fd, i;
	char my_buf[4000];

	unsigned int size;
	char c;

	/* fill my buffer with *'s */
	for(i=0; i<4000; i++) my_buf[i] = 'n';

	/* open the device for read/write/lseek */
	printf("[%d] - Opening device rsa_mem_cdev\n", getpid() );
	fd = open( "/dev/rsa_mem_cdev", O_RDWR );
	if( fd < 0 ) {
		printf("\n\nDevice could not be opened\n\n");
		return 1;
	}
	printf("Device opened with ID [%d]\n", fd);

	/* write the contents of my buffer into the device */
	write( fd, my_buf, 4000 );

	/* empty my buffer now */
	bzero(my_buf, 4000);

	/* read 70 characters */
	lseek( fd, 20, SEEK_SET );
	read( fd, my_buf, 70 );
	printf("I read this from the device\n%s\n", my_buf);

	/* empty by buffer again */
	bzero(my_buf, 4000);

	/* Write something into the buffer */
	write( fd, "TADAAA", 14);

	/* Set ppos to position 60 */
	lseek( fd, 60, SEEK_SET );

	/* read 70 characters and print */
	read( fd, my_buf, 70 );
	printf("I read this from the device\n%s\n", my_buf);

	/* Close the device */
	close(fd);

	exit(0);
}


