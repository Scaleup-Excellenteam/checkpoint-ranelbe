#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ranel");
MODULE_DESCRIPTION("A netfilter kernel module that logs incoming packets info.");
MODULE_VERSION("0.01");

static struct nf_hook_ops *nfho;

// Netfilter hook function
unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    // Log the source and destination IP addresses
    printk(KERN_INFO "Hello world\n");
    // Accept the packet
    return NF_ACCEPT;
}

static int __init ex2_init(void)
{
    // Allocate memory for the nf_hook_ops object
    nfho = (struct nf_hook_ops *)kcalloc(1, sizeof(struct nf_hook_ops), GFP_KERNEL);

    if (!nfho) {
        printk(KERN_ERR "Failed to allocate memory for nf_hook_ops.\n");
        return -ENOMEM;
    }

    // Initialize netfilter hook
    nfho->hook = (nf_hookfn *)hook_func;
    nfho->hooknum = NF_INET_PRE_ROUTING;  // Intercept incoming packets
    nfho->pf = PF_INET;                  // IPv4 packets
    nfho->priority = NF_IP_PRI_FIRST;    // Set the highest priority
    nf_register_net_hook(&init_net, nfho);  // Register the netfilter hook

    return 0;
}

static void __exit ex2_exit(void)
{
    nf_unregister_net_hook(&init_net, nfho);
    kfree(nfho); // Free the allocated memory
}

module_init(ex2_init);
module_exit(ex2_exit);
