/*
 *	usb-key-input-app 
 *	author : zyy
 *	date   : 20210420
 *	/dev/input/by-id/usb-13ba_0001-event-kbd
 * */

#include <stdint.h>
#include <linux/input.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/file.h>

struct key_code{
    const char *name;
    unsigned short code;
};

struct key_code  ev_msc[] = {
{"NUM"  , 0x53},
{"*"    , 0X55},
{"7"    , 0X5F},
{"9"    , 0X61},
{"4"    , 0X5C},
{"6"    , 0X5E},
{"1"    , 0X59},
{"3"    , 0X5B},
{"0"    , 0X62},
{"/"    , 0X54},
{"-"    , 0X56},
{"8"    , 0X60},
{"+"    , 0X57},
{"5"    , 0X5D},
{"BACK" , 0X2A},
{"2"    , 0X5A},
{"."    , 0X63},
{"enter", 0X58}
};

struct key_code ev_key[]={
	{"0"    ,  82},
	{"1"    ,  79},
	{"2"    ,  80},
	{"3"    ,  81},
	{"4"    ,  75},
	{"5"    ,  76},
	{"6"    ,  77},
	{"7"    ,  71},
	{"8"    ,  72},
	{"9"    ,  73},
	{"NUM"  ,  69},
	{"/"    ,  98},
	{"*"    ,  55},
	{"-"    ,  74},
	{"+"    ,  78},
	{"enter",  96},
	{"."    ,  83}
};

#define USB_KEY_TYPE 1

int main(int argc, char *argv[])
{
    struct input_event ev;
    int fd, rd;
    int i, ev_msc_size,ev_key_size;
    char *filename;
#if 1
    if(argc!=2){
    		printf("error usage \r\n");
	    return -1;
    }

    filename = argv[1];
#endif
    //Open Device
    if((fd = open(filename, O_RDONLY)) == -1)
    //if((fd= open("/dev/input/by-id/usb-13ba_0001-event-kbd",O_RDONLY)) == -1)
    {
        printf("Not a vaild device.\n");
        return -1;
    }

    ev_msc_size = sizeof(ev_msc)/sizeof(ev_msc[0]);
    ev_key_size = sizeof(ev_key)/sizeof(ev_key[0]);

	 //printf("ev_msc_size :%d\r\n",ev_msc_size);
	 //printf("ev_key_size :%d\r\n",ev_key_size);

    while(1)
    {
        memset((void *)&ev, 0, sizeof(ev));

        rd = read(fd, (void *)&ev, sizeof(ev));

        if(rd <= 0)
        {
            printf("rd: %d\n", rd);
        }

#if USB_KEY_TYPE 
	if(ev.type == 1){
		switch(ev.value)
		{
			case 0:
				printf("key release \n");
				break;
			case 1:
				printf("key press \n");
				break;
			case 2:
				printf("key hold \n");
				break;
		}

		//printf("code : %d \n",ev.code);	
		for(i=0;i<ev_key_size;i++)
		{	
			if(ev.code == ev_key[i].code){
				printf("%s\n",ev_key[i].name);
				break;
			}
		}
	}
#else
	if(ev.type ==1){
		switch(ev.value)
		{
			case 0:
				printf("key release \n");
				break;
			case 1:
				printf("key press \n");
				break;
			case 2:
				printf("key hold \n");
				break;
		}
	}
        if(ev.type == 4)
        {
            //printf("value: %x\n", ev.value&0xff);
	    for(i=0;i<ev_msc_size;i++){
		if((ev.value&0xff) == ev_msc[i].code)
	    	{
			printf("%s\n",ev_msc[i].name);
			break;
		}
	    }
        }
#endif

    }
    return 0;
}

