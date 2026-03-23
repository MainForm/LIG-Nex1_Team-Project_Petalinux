// SPDX-License-Identifier: GPL-2.0-only
#include <linux/module.h>
#include <linux/init.h>
 
static int __init initModule(void){
    printk("[devModule] initModule : Hello world\n");
 
    return 0;
}
 
static void __exit exitModule(void){
    printk("[devModule] exitModule : Goodbye world\n");
}
 
module_init(initModule);
module_exit(exitModule);
 
MODULE_LICENSE("GPL");
