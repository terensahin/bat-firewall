#include <linux/atomic.h> 
#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/module.h>
#include <linux/printk.h> 
#include <linux/types.h> 
#include <linux/uaccess.h> /* for get_user and put_user */ 
#include <linux/version.h> 
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/string.h>
#include <linux/inet.h>
#include <asm/errno.h> 
#include "../common.h"
#include "../chardev.h" 

#define SUCCESS 0 
#define DEVICE_NAME "char_dev" 
 
enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
}; 

firewall_rule firewall_rules[32];
 
/* Is the device open right now? Used to prevent concurrent access into 
 * the same device 
 */ 
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 
 
static struct class *cls; 

static struct nf_hook_ops *nfho = NULL;
 
/* This is called whenever a process attempts to open the device file */ 
static int device_open(struct inode *inode, struct file *file) 
{ 
    pr_info("device_open(%p)\n", file); 
 
    try_module_get(THIS_MODULE); 
    return SUCCESS; 
} 
 
static int device_release(struct inode *inode, struct file *file) 
{ 
    pr_info("device_release(%p,%p)\n", inode, file); 
 
    module_put(THIS_MODULE); 
    return SUCCESS; 
} 
 
/* This function is called whenever a process tries to do an ioctl on our 
 * device file. We get two extra parameters (additional to the inode and file 
 * structures, which all device functions get): the number of the ioctl called 
 * and the parameter given to the ioctl function. 
 * 
 * If the ioctl is write or read/write (meaning output is returned to the 
 * calling process), the ioctl call returns the output of this function. 
 */ 
static long device_ioctl(struct file *file,
             unsigned int ioctl_num, /* command */ 
             unsigned long ioctl_param) 
{ 
    long ret = SUCCESS; 
 
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 
 
    switch (ioctl_num) { 
        case IOCTL_SET_MSG: {
            firewall_rule __user *user_frule_ptr = (firewall_rule __user *)ioctl_param;

            if (copy_from_user(firewall_rules, user_frule_ptr, sizeof(firewall_rules))) {
                pr_info("error COPY");
                return -EFAULT;
            }

            pr_info("%s %d %s", firewall_rules[0].address, firewall_rules[0].port, firewall_rules[0].protocol);
            pr_info("%s %d %s", firewall_rules[1].address, firewall_rules[1].port, firewall_rules[1].protocol);
            pr_info("%s %d %s", firewall_rules[2].address, firewall_rules[2].port, firewall_rules[2].protocol);

            break;
        }
    } 
 
    atomic_set(&already_open, CDEV_NOT_USED); 
 
    return ret; 
} 
 
/* Module Declarations */ 
static struct file_operations fops = { 
    .unlocked_ioctl = device_ioctl, 
    .open = device_open, 
    .release = device_release,
}; 


static int isBlocked(int port, uint32_t address, char* protocol){

    uint8_t tmp_addr[4];
    uint32_t addr;

    for(int i = 0; i < 8; i++){

        pr_info("check firewall_rule: %s\n", firewall_rules[i].address);

        if(in4_pton(firewall_rules[i].address, -1, tmp_addr, -1, NULL) <= 0){
            pr_info("INT4 PTON ERROR\n");
            continue;
        }

        addr = (tmp_addr[0] << 24) | (tmp_addr[1] << 16) | (tmp_addr[2] << 8) | tmp_addr[3];

        if (addr != address){
            continue;
        }

        if (port != firewall_rules[i].port){
            continue;
        }

        if (strncmp(protocol, firewall_rules[i].protocol, 3) != 0){
            continue;
        }

        return 1;
    }
    
    return 0;
}

static unsigned int hfunc(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;
    struct udphdr *udp_header;
    uint32_t src_ip_address;
    if (!skb)
        return NF_ACCEPT;

    ip_header = ip_hdr(skb);
	
    src_ip_address = ntohl(ip_header->saddr);

    pr_info("SOURCE IP ADDRESS %d\n", src_ip_address); 
    if (ip_header->protocol == IPPROTO_TCP) {

        printk(KERN_INFO "TCP packet detected!\n");
        tcp_header = (struct tcphdr *) skb_transport_header(skb);

        if (isBlocked(ntohs(tcp_header->dest), src_ip_address, "tcp") == 1){
            pr_info("tcp dropped\n"); 
            return NF_ACCEPT;
        } 

    }else if (ip_header->protocol == IPPROTO_UDP) {
        printk(KERN_INFO "UDP packet detected!\n");
        
	    udp_header = udp_hdr(skb);
        if (isBlocked(ntohs(udp_header->dest), src_ip_address, "udp") == 1){
            pr_info("udp dropped\n");
            return NF_ACCEPT;
        }
		
	}
    
    pr_info("packet accepted\n");
    return NF_ACCEPT;
}




 
/* Initialize the module - Register the character device */ 
static int __init chardev2_init(void) 
{ 
    /* Register the character device (atleast try) */ 
    int ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops); 
 
    /* Negative values signify an error */ 
    if (ret_val < 0) { 
        pr_alert("%s failed with %d\n", 
                 "Sorry, registering the character device ", ret_val); 
        return ret_val; 
    } 
 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0) 
    cls = class_create(DEVICE_FILE_NAME); 
#else 
    cls = class_create(THIS_MODULE, DEVICE_FILE_NAME); 
#endif 
    device_create(cls, NULL, MKDEV(MAJOR_NUM, 0), NULL, DEVICE_FILE_NAME); 
 
    pr_info("Device created on /dev/%s\n", DEVICE_FILE_NAME); 


    nfho = (struct nf_hook_ops*)kcalloc(1, sizeof(struct nf_hook_ops), GFP_KERNEL);
    
    /* Initialize netfilter hook */
    nfho->hook  = (nf_hookfn*)hfunc;        /* hook function */
    nfho->hooknum   = NF_INET_PRE_ROUTING;      /* received packets */
    nfho->pf    = PF_INET;          /* IPv4 */
    nfho->priority  = NF_IP_PRI_FIRST;      /* max hook priority */
    
    nf_register_net_hook(&init_net, nfho);

 
    return 0; 
} 
 
/* Cleanup - unregister the appropriate file from /proc */ 
static void __exit chardev2_exit(void) 
{ 
    device_destroy(cls, MKDEV(MAJOR_NUM, 0)); 
    class_destroy(cls); 
 
    /* Unregister the device */ 
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME); 

    nf_unregister_net_hook(&init_net, nfho);
    kfree(nfho);
} 
 
module_init(chardev2_init); 
module_exit(chardev2_exit); 
 
MODULE_LICENSE("GPL");
