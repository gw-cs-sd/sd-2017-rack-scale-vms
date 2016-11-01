Install Xen 4.7
==

About
--

This guide describes how to install and configure Xen 4.7 with support
for [XSM-FLASK][xsm], [Xen AltP2m with Single-Step Monitoring][altp2m], and [HVM guests][hvm].

After setting up Xen, this will guide will step through creating an HVM
DomU set up for introspection using [Bitdefender's virtual machine
introspection library][libbdvmi] written by [Razvan
Cojocaru][rcojocaru].

This guide assumes [Ubuntu 14.04][ubuntu] or [Debian Jessie][debian] for both the Dom0 OS
and the DomU OS.

Install Xen
--
This will install Xen 4.7 with Dom0 configured such that it has 4Gb RAM,
two assigned CPU cores, support for altp2m, and support for XSM Flask
Enforcing.

This guide is highly based off of [Drakvuf's Xen Install Guide][drakvuf]
written by [Tamas K Lengyel][tklengyel].

```
sudo apt-get update && sudo apt-get upgrade -y
sudo apt-get install software-properties-common
sudo add-apt-repository -y ppa:danielrichter2007/grub-customizer
sudo apt-get update
sudo apt-get install grub-customizer
sudo apt-get install wget git bcc bin86 gawk bridge-utils iproute libcurl3 libcurl4-openssl-dev bzip2 module-init-tools pciutils-dev build-essential make gcc clang libc6-dev libc6-dev-i386 linux-libc-dev zlib1g-dev python python-dev python-twisted python-gevent python-setuptools libncurses5-dev patch libvncserver-dev libssl-dev libsdl-dev iasl libbz2-dev e2fslibs-dev git-core uuid-dev ocaml libx11-dev bison flex ocaml-findlib xz-utils gettext libyajl-dev libpixman-1-dev libaio-dev libfdt-dev cabextract libglib2.0-dev autoconf automake libtool check libjson-c-dev libfuse-dev libsystemd-daemon-dev checkpolicy
git clone https://github.com/gw-cs-sd/sd-2017-rack-scale-vms.git
cd sd-2017-rack-scale-vms
git submodule update --init
cd deps/xen-drakvuf
./configure --enable-githttp
make -j4 dist-xen
make -j4 dist-tools
sudo su
make -j4 install-xen
make -j4 install-tools
echo "GRUB_CMDLINE_XEN_DEFAULT=\"dom0_mem=4096M,max:4096M dom0_max_vcpus=4 dom0_vcpus_pin=true hap_1gb=false hap_2mb=false altp2m=1 flask_enforcing=1\"" >> /etc/default/grub
echo "/usr/local/lib" > /etc/ld.so.conf.d/xen.conf
ldconfig
echo "none /proc/xen xenfs defaults,nofail 0 0" >> /etc/fstab
echo "xen-evtchn" >> /etc/modules
echo "xen-privcmd" >> /etc/modules
update-rc.d xencommons defaults 19 18
update-rc.d xendomains defaults 21 20
update-rc.d xen-watchdog defaults 22 23
```

Edit the `/etc/grub.d/20_linux_xen` file and add the following line
after the entry that says `multiboot`.  This line should be the last
module to be loaded.  See the [example 20_linux_xen][20_linux_xen] file
for clarification.
  - `module ${rel_dirname}/xenpolicy-4.7.0`

Finally, assure GRUB is configured properly to boot the Xen Kernel.
First, SSH into the server assuring X11 forwarding.  Then, run the
command:
  - `sudo grub-customizer`

And select the correct entry that boots Linux with Xen.  Save that as
the primary boot entry in GRUB.

Next, reboot the machine.

Once the machine is back up, we need to create the Xen network bridge.  To do so, make sure the `/etc/network/interface` file looks like the following for the primary network interface (See this [network config][netiface] for an example):

```
# The loopback network interface
auto lo
iface lo inet loopback

# The primary network interface
iface em1 inet manual.

auto xenbr0
iface xenbr0 inet dhcp
    bridge_ports em1
```

Once the network has been configured for the Xen bridge, restart the machine.

Now, Xen should be installed and configured.  To test this, run the
following commands to assure that Xen is working and Dom0 is configured
as intended:
  - `sudo xen-detect`
  - `sudo xl li -l`
  - `sudo xl dmesg`

Create HVM DomU
--

This guide steps through creating an HVM Ubuntu 14.04 DomU with a 11Gb disk setup with AltP2m support, XSM-FLASK support, and a VNC console.

  - Create the image file
    + `dd if=/dev/zero of=/ubuntu-hvm.img bs=1024k seek=11264 count=0`
  - Download an [Ubuntu Server 14.04 installation iso][ubuntu-iso]
  - Use the [ubuntu-hvm][domu] config file as a starting point.  This config file assumes installation through the Ubuntu 14.04 installation image
  - Create a DomU instance to install Ubuntu from the installation file into the image
    + `sudo xl create ubuntu-hvm.cfg`
  - Attach to the DomU's console via VNC.  Assuming an SSH connection to the Dom0 host with X11 forwarding enabled
    + `sudo vncviewer localhost`
  - Follow the installation guide in the VM to setup Ubuntu 14.04 server
  - Once installed, destroy the VM and modify the DomU config file, `ubuntu-hvm.cfg` such that it does not boot from the CD drive
    + `sudo xl destroy ubuntu-hvm`
    + `vim ubuntu-hvm.cfg`
      - Change the `disk` line to remove the cdrom
      - Change the `boot` line to `boot="cd"`
  - Create a new instance of DomU and expect it to be an installed Ubuntu Server 14.04 instance
    + `sudo xl create ubuntu-hvm.cfg`

[xsm]: https://wiki.xenproject.org/wiki/Xen_Security_Modules_:_XSM-FLASK#Enabling_XSM_in_Xen
[altp2m]: https://blog.xenproject.org/2016/04/13/stealthy-monitoring-with-xen-altp2m/
[hvm]: https://wiki.xen.org/wiki/Xen_Project_Software_Overview#HVM
[libbdvmi]: https://blog.xenproject.org/author/rc/
[rcojocaru]: https://github.com/razvan-cojocaru
[ubuntu]: https://wiki.ubuntu.com/TrustyTahr/ReleaseNotes
[debian]: https://www.debian.org/releases/jessie/
[drakvuf]: https://drakvuf.com/
[tklengyel]: https://tklengyel.com/
[20_linux_xen]: configs/20_linux_xen
[ubuntu-iso]: http://releases.ubuntu.com/14.04/ubuntu-14.04.5-server-amd64.iso
[domu]: configs/ubuntu-hvm.cfg
[netiface]: configs/interfaces
