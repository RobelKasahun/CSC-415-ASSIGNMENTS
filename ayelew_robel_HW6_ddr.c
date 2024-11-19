/**************************************************************
 * Class::  CSC-415-02 Spring 2024
 * Name:: Robel Ayelew
 * Student ID:: 922419937
 * GitHub Name:: RobelKasahun
 * Project:: Assignment 6 – Device Driver
 *
 * File:: ayelew_robel_HW6_ddr
 *
 * Description:: This is a device driver assignment that takes
 * a string and encryptes the string and accepts a encrypted message
 * and returns or displays the original string
 *
 **************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include <linux/device.h>
#include <linux/blkdev.h>

#define DEVICE_NAME "ayelew_robel_HW6_ddr"
#define BUFFER_SIZE 2008
#define MY_MAJOR 615
#define MY_MINOR 0
#define FAILURE -1
#define SUCCESS 0
#define DRIVER_AUTHOR "Robel K Ayelew"
#define DRIVER_DESCRIPTION "This is a device driver assignment that encrypts and decrypts a string"
#define LICENSE "GPL"
#define ECRYPTION_AND_DECRYPTION_KEY 0X16

static char message[BUFFER_SIZE];
struct cdev my_cdev;
static int registration_code;

/** Function Prototypes */
static int my_device_open(struct inode *, struct file *);
static int my_device_release(struct inode *, struct file *);
static ssize_t my_device_write(struct file *, const char *, size_t, loff_t *);
static ssize_t my_device_read(struct file *, char *, size_t, loff_t *);
static long my_io_ctrl(struct file *fs, unsigned int command, unsigned long arg);
static void encrypt_message(const char *, char *, size_t);
static void decrypt_message(char *, char *, size_t);

// file operations
static struct file_operations fops = {
    .open = my_device_open,
    .release = my_device_release,
    .read = my_device_read,
    .write = my_device_write,
    .unlocked_ioctl = my_io_ctrl,
    .owner = THIS_MODULE,
};

// initialize and load the module
int init_module(void)
{
    int result;
    dev_t dev_number = MKDEV(MY_MAJOR, MY_MINOR);
    // register the device
    registration_code = register_blkdev(MY_MAJOR, DEVICE_NAME);

    if (registration_code < 0)
    {
        printk(KERN_INFO "*** Device registration has been failed with result code of %d ***\n", registration_code);
        return registration_code;
    }

    // initialize cdev
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    // add my_cdev to the system
    result = cdev_add(&my_cdev, dev_number, 1);

    if (result < 0)
    {
        // failed to add my_cdev to the system
        printk(KERN_INFO "*** chardev registration has been failed. ***\n");
        return result;
    }

    printk(KERN_INFO "*** Device registration has been successful with result code of %d ***\n", registration_code);

    return SUCCESS;
}

// unregistering and removing device from kernel
void cleanup_module(void)
{
    // unregister the device that registered by the init_module
    unregister_chrdev(MY_MAJOR, DEVICE_NAME);
    printk(KERN_INFO "*** The device named %s has been successfully unregistered ***\n", DEVICE_NAME);
}

/** Function definitions */
static int my_device_open(struct inode *inode, struct file *file)
{
    // Allocate virtual memory from the kernel
    struct file *temp_file = vmalloc(sizeof(struct file));
    if (!temp_file)
    {
        printk(KERN_INFO "*** failed to allocate virtual memory ***\n");
        return FAILURE;
    }

    // set file->private_data to temp_file
    file->private_data = temp_file;
    return SUCCESS;
}

static int my_device_release(struct inode *inode, struct file *file)
{
    // release a dynamically allocated virtual storage
    struct file *temp_file = (struct file *)file->private_data;
    vfree(temp_file);
    return 0;
}

static ssize_t my_device_read(struct file *filep, char *buffer, size_t size, loff_t *offset)
{

    char decrypted_message[BUFFER_SIZE] = {0};

    size_t bytes;

    if (BUFFER_SIZE > size)
    {
        bytes = size;
    }
    else
    {
        bytes = BUFFER_SIZE;
    }

    // decrypt the message
    decrypt_message(message, decrypted_message, bytes);

    // number of bytes that could not be copied
    return copy_to_user(buffer, encrypt_message, bytes);
    // return bytes; // returns the bytes that will be copied
}

static ssize_t my_device_write(struct file *filep, const char *buffer, size_t size, loff_t *off)
{
    size_t bytes;

    if (BUFFER_SIZE > size)
    {
        bytes = size;
    }
    else
    {
        bytes = BUFFER_SIZE;
    }

    char encryptedMessage[BUFFER_SIZE] = {0};
    // encrypt the message
    encrypt_message(buffer, encryptedMessage, bytes);
    // copy bytes of data from the encrypted message to the message
    strncpy(message, encryptedMessage, bytes);

    // return the copied bytes
    return bytes;
}

static long my_io_ctrl(struct file *fs, unsigned int command, unsigned long arg)
{
    return 0;
}

// encrypt the message
static void encrypt_message(const char *input_message, char *output_message, size_t size)
{
    // loop over every character of the input_message
    int index = 0;
    while (index < size)
    {
        // apply Bitwise operation for each character of input_message
        // assign the result to the output_message
        output_message[index] = input_message[index] ^ ECRYPTION_AND_DECRYPTION_KEY;
        ++index;
    }
}

// decrypt the encrypted message
static void decrypt_message(char *input_message, char *output_message, size_t size)
{
    // loop over every character of the input_message
    int index = 0;
    while (index < size)
    {
        // apply Bitwise operation for each character of input_message
        // assign the result to the output_message
        output_message[index] = input_message[index] ^ ECRYPTION_AND_DECRYPTION_KEY;
        ++index;
    }
}

MODULE_LICENSE(LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);