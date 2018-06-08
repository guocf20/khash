/*---------------------
* Copyright(C)2018 All rights reserved
*author:guochengfeng
*last modify:2018--06--08
*email:guocf20@gmail.com
*=================================*/
#include<stdio.h>
#include<sys/ioctl.h>
#include<fcntl.h>
#include<string.h>
#define IOCTL_SET 102

void main(int argc, char **argv)
{
    int fd = open("/dev/test", O_RDWR);
    char buf[128] = {'\0'};
    if(fd < 0)
    {
        perror("open");
    }
    snprintf(buf, 127, "%s", argv[1]);
    ioctl(fd, 102, buf);

    printf("%s\n", buf);
    close(fd);
    return 0;
}
