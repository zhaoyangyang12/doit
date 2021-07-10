#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
//#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include "oled128x32_font.h"

#define DEV_CNT  1
#define DEV_NAME "oled"
#define OLED_CMD  0x00  //OLED写命令
#define OLED_DATA 0x40  //OLED写数据

#define Max_Column  128

struct oled_dev {
    dev_t     devid;
    int       major;              
    int       minor;
    struct    cdev cdev;          
    struct    class *class;   
    struct    device *device;                      
    void      *private_data;       
};

static struct oled_dev oled;

static s32 oled_write_byte(u8 reg, u8 para, u8 len)

{
    u8 data[2];
    struct i2c_msg msg;
    struct i2c_client *client = (struct i2c_client *)oled.private_data;
	    				//此处将.probe函数中所保存的私有数据强制转换为i2c_client结构体
    data[0] = reg;					//寄存器
    data[1] = para;					//参数
    msg.addr = client->addr;				//ap3216c地址, 设备树中的地址
    msg.flags = 0;					//标记为写
    msg.buf = data;					//要写入的数据缓冲区
    msg.len = len + 1;					//要写入的数据长度
    return i2c_transfer(client->adapter, &msg, 1);	//用于发送的client
}

void oled_init(void)
{
    u8 i;
    u8 data[] ={ 0xAE, 0x00, 0x10, 0x40, 0xB0, 0x81, 0xFF, 0xA1, 0xA6,
                 0xA8, 0x3F, 0xC8, 0xD3, 0x00, 0xD5, 0x80, 0xD8, 0x05,
                 0xD9, 0xF1, 0xDA, 0x12, 0xDB, 0x30, 0x8D, 0x14, 0xAF };
    for(i=0; i<sizeof(data); i++)
    {
       oled_write_byte(OLED_CMD, data[i], 1);
    }
}

void oled_clear(void)
{  
    u8 i, n;
    for(i=0; i<8; i++) 
    {  
       oled_write_byte(OLED_CMD, 0xb0 + i, 1);      //设置页地址（0~7）
       oled_write_byte(OLED_CMD, 0x00,   1);        //设置显示位置—列低地址
       oled_write_byte(OLED_CMD, 0x10,   1);        //设置显示位置—列高地址
       for(n=0; n<128; n++)
       {
           oled_write_byte(OLED_DATA, 0x00, 1);
       }
    }
}

void oled_set_pos(u8 x, u8 y)
{ 
    oled_write_byte(OLED_CMD, 0xb0 + y, 1);
    oled_write_byte(OLED_CMD, ((x & 0xf0) >> 4) | 0x10, 1);
    oled_write_byte(OLED_CMD, x & 0x0f, 1);
}

void oled_showchar(u8 x, u8 y, u8 chr)
{     
    u8 c=0, i=0;
    c = chr - ' ';                    
    if(x > Max_Column-1)
    {
       x = 0;
       y = y + 2;
    }
    oled_set_pos(x, y);
    for(i=0; i<8; i++)
    {
       oled_write_byte(OLED_DATA, F8X16[c*16+i], 1);
    }
    oled_set_pos(x,y+1);
    
    for(i=0; i<8; i++)
    {
       oled_write_byte(OLED_DATA, F8X16[c*16+i+8], 1);  
    }
}

void oled_showstring(u8 x, u8 y, u8 *chr)
{
    unsigned char j=0;
   
    while(chr[j] != '\0')
    {     
       oled_showchar(x, y, chr[j]);
       x += 8;
       if(x > 120)
       {
           x = 0;
           y += 2;
       }
       j++;
    }
}

static int oled_open(struct inode *inode, struct file *filp)
{
    oled_init();
    oled_clear();
    return 0;
}

struct display_stru{
    int  x;
    int  y;
    char *buf;
};

static ssize_t oled_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *off)
{
    int ret;
    struct display_stru dis_format;
    ret = copy_from_user(&dis_format, buf, cnt);
    printk("dis_format.x = %d \r\n", dis_format.x);		//调试打印，用于观察从用户控件获取的数据是否正确，可删除
    printk("dis_format.y = %d \r\n", dis_format.y);
    printk("dis_format.buf = %s \r\n", dis_format.buf);
    oled_showstring(dis_format.x, dis_format.y, dis_format.buf);
    return 0;
}

static int oled_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static struct file_operations oled_ops = {
    .owner   = THIS_MODULE,
    .open    = oled_open,
    .write   = oled_write,
    .release = oled_release,
};

static int oled_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    printk("oled_probe \r\n");

    if(oled.major)
    {
       oled.devid = MKDEV(oled.major, 0);
       register_chrdev_region(oled.devid, DEV_CNT, DEV_NAME);
    }
    else
    {
       alloc_chrdev_region(&oled.devid, 0, DEV_CNT, DEV_NAME);
       oled.major = MAJOR(oled.devid);
       oled.minor = MINOR(oled.devid);
    }
    printk("oled major = %d, minor = %d \r\n", oled.major, oled.minor);
 
    cdev_init(&oled.cdev, &oled_ops);						//初始化字符设备并向Linux内核添加
    cdev_add(&oled.cdev, oled.devid, DEV_CNT);
    oled.class = class_create(THIS_MODULE, DEV_NAME);				//创建设备类
    if(IS_ERR(oled.class))
    {
       return PTR_ERR(oled.class);
    }
    oled.device = device_create(oled.class, NULL, oled.devid, NULL, DEV_NAME);		//创建设备
	
    if(IS_ERR(oled.device))
    {
       return PTR_ERR(oled.device);
    }
    oled.private_data = client;
    return 0;
}

static int oled_remove(struct i2c_client *client)
{
    printk("oled_remove \r\n");
    cdev_del(&oled.cdev);				//注销字符设备
    unregister_chrdev_region(oled.devid, DEV_CNT);        //注销设备号
    device_destroy(oled.class, oled.devid);               //注销设备节点
    class_destroy(oled.class);                            //注销设备类
    return 0;
}

static const struct i2c_device_id oled_id[] = {
    {"alientek,oled", 0}, 
    {  }
};

static const struct of_device_id oled_of_match[] = {
    { .compatible = "alientek,oled" },
    {  }
};

static struct i2c_driver oled_driver = {
    .probe  = oled_probe,
    .remove = oled_remove,
    .id_table = oled_id,
    .driver = {
          .owner = THIS_MODULE,
          .name = "oled",
          .of_match_table = oled_of_match,
          },
};

static int __init oled_module_init(void)
{
    int ret = 0;
    printk("oled_module_init \r\n");
    ret = i2c_add_driver(&oled_driver);
    return ret;
}

static void __exit oled_module_exit(void)
{
    printk("oled_module_exit \r\n");
    i2c_del_driver(&oled_driver);
}

module_init(oled_module_init);
module_exit(oled_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LMENG");
