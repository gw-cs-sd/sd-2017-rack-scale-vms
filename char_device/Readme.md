## Setup

Make sure you have all the kernel headers installed

```sudo apt-get install build-essential linux-headers-$(uname -r)```


## Registering the char device

`make` should generate all the files needed for the module

```sudo insmod chr_drv_udev.ko```

should put your module in the kernel. Confirm this by running `lsmod`.
You should be seeing `chr_drv_udev`

The device should be `/dev/rsa_mem_cdev`

To test the device, run

```gcc test_chrdev.c -o test && sudo ./test```
