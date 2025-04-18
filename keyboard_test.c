#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

static int fd=0;

unsigned int crc_chk(unsigned char* data, unsigned char length)
{
    int j;
    unsigned int reg_crc=0XFFFF;		//CRC寄存器赋初值
    while(length--)	//遍历所有的指令信息
    {
        reg_crc ^= *data++;		//异或运算
        for (j=0;j<8;j++)		//依次检查每个字节的8个位为1还是为0
        {
            if (reg_crc & 0x01) 
             {
                //最低位为1则与0xA001进行异或运算
                reg_crc=(reg_crc>>1) ^ 0xA001; 	
            }
        else 
        {
                //最低位为0则不处理
                reg_crc=reg_crc >>1;
        }
        }
    }
    return reg_crc;	 //最后返回CRC寄存器的值
}

// 打开串口
int open_serial_port(const char *port) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Unable to open serial port");
        return -1;
    }

    // 配置串口
    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B115200);  // 输入波特率
    cfsetospeed(&options, B115200);  // 输出波特率
    options.c_cflag |= (CLOCAL | CREAD);  // 启用接收
    options.c_cflag &= ~PARENB;  // 无奇偶校验
    options.c_cflag &= ~CSTOPB;  // 1 位停止位
    options.c_cflag &= ~CSIZE;   // 清除数据位掩码
    options.c_cflag |= CS8;      // 8 位数据位
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // 原始模式
    options.c_iflag &= ~(IXON | IXOFF | IXANY);  // 禁用流控制
    options.c_oflag &= ~OPOST;  // 原始输出模式

    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

// 发送数据
int send_data(int fd, const unsigned char *data, size_t length) {
    int ret = 0;
    // 确保总线空闲时间 ≥ 3.5 个字符时间（304 µs）
    usleep(350);  // 350 µs 确保足够

    ssize_t written = write(fd, data, length);
    if (written < 0) {
        ret = errno;
        perror("Write failed");
    } else {
        printf("Sent %zd bytes: ", written);
        for (size_t i = 0; i < length; i++) {
            printf("%02X ", data[i]);
        }
        printf("\n");
    }

    // 确保总线空闲时间 ≥ 3.5 个字符时间（304 µs）
    usleep(350);  // 350 µs 确保足够
    return ret;
}

// 接收数据
int receive_data(int fd) {
    unsigned char buffer[256];
    int ret = 0;
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        ret = errno;
        perror("Read failed");
    } else if (bytes_read == 0) {
        printf("No data received\n");
    } else {
        buffer[bytes_read] = '\0';  // 确保字符串以 NULL 结尾
        printf("Rece %zd bytes: ", bytes_read);
        for (ssize_t i = 0; i < bytes_read; i++) {
            printf("%02X ", buffer[i]);
        }
        printf("\n");
    }
    return ret;
}

int kb_data_send(unsigned char *data, size_t length)
{
    int ret = 0;
    ret = send_data(fd, data, length);
    if(ret == 0)
    {
        usleep(20000);
        ret = receive_data(fd);
        printf("\n\n");
    }
    return ret;

}

int kb_register_write(uint16_t address,uint16_t data)
{
     unsigned char buff[8];

     buff[0]=0x01;
     buff[1]=0x06;
     buff[2]=address>>8;
     buff[3]=address;
     buff[4]=data>>8;
     buff[5]=data;
     int crc=crc_chk(buff, 6);
     buff[6]=crc;
     buff[7]=crc>>8;

     returnkb_data_send(buff, 8);
}

int kb_register_read(uint16_t address,uint16_t len)
{
     unsigned char buff[8];

     buff[0]=0x01;
     buff[1]=0x03;
     buff[2]=address>>8;
     buff[3]=address;
     buff[4]=len>>8;
     buff[5]=len;
     int crc=crc_chk(buff, 6);
     buff[6]=crc;
     buff[7]=crc>>8;

     return kb_data_send(buff, 8);
}

void get_kb_version(void)
{
    kb_register_read(0x0001,1);
}

void set_lcd_onoff(uint16_t onoff)
{
    uint16_t add=0x0002;
    uint16_t val=onoff;
    kb_register_write(add,val);
}

void set_lcd_level(uint8_t level){
    uint16_t value = 10;
    value = value*level;
    printf("set led level:%d.\n",value);
    kb_register_write(0x0003,value);
    usleep(200000);      
}

void set_led_color(uint16_t c_r,uint16_t c_gb){
    uint16_t add_l=0x0004;
    uint16_t add_h=0x0005;
    
    for(int i=0;i<30;i++)
    {
        printf("set led%d.\n",i+1);
        add_l=0x0004+(i*2);
        add_h=0x0005+(i*2);

        kb_register_write(add_l,c_r);
        kb_register_write(add_h,c_gb); 
        usleep(20000);      
    }
}

void set_single_led_color(uint8_t led, uint16_t c_r,uint16_t c_gb){
    uint16_t add_l=0x0004+(led*2);
    uint16_t add_h=0x0005+(led*2);

    kb_register_write(add_l,c_r);
    kb_register_write(add_h,c_gb);
    printf("set led%d.\n",led+1);
    usleep(20000); 
}

void set_led_blue(uint16_t c_r,uint16_t c_gb)
{
    uint16_t add_l=0x0004;
    uint16_t add_h=0x0005;
    
    for(int i=0;i<30;i++)
    {
        printf("set led%d.\n",i+1);
        add_l=0x0004+(i*2);
        add_h=0x0005+(i*2);

        kb_register_write(add_l,c_r);
        kb_register_write(add_h,c_gb); 
        usleep(20000);      
    }
}

int keyboard_init(void){
    static uint8_t init_flag= 0;
    if(init_flag)
         return EXIT_SUCCESS;
         
    const char *port = "/dev/ttyS7";  // 根据实际串口设备修改
    fd = open_serial_port(port);
    if (fd == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int keyboard_exit(void){
    return close(fd);
}

int get_kb_status(void){
   return kb_register_read(0x0002,1);

}

int keyboard_test(void){
    const char *port = "/dev/ttyS7";  // 根据实际串口设备修改
    fd = open_serial_port(port);
    if (fd == -1) {
        return EXIT_FAILURE;
    }

    // 要发送的数据组
    //unsigned char data[] = {0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0xD5, 0xCA};  // Modbus RTU 示例帧
    //unsigned char data[] = {0x01 ,0x10 ,0x00 ,0x64 ,0x00 ,0x04 ,0x08 ,0x00 ,0x0A ,0x00 ,0x64 ,0x03 ,0xE8 ,0x27 ,0x10 ,0x86 ,0xB1};

    // 发送数据
    // send_data(fd, data, sizeof(data));
    // usleep(300000);
    // 接收数据
    // receive_data(fd);

    kb_register_write(0x0002,0);
    usleep(300000);
    receive_data(fd);
    usleep(300000);
    kb_register_write(0x0002,1);
    usleep(300000);
    receive_data(fd);

    // 关闭串口
    close(fd);
    return EXIT_SUCCESS;
}

void change_keyled_stat(void) {
    static char led_status=0;
    if (led_status == 0) { // 如果 LED 是关闭状态
          set_led_color(0x002F,0x2F2F);
          led_status=1;
    } else {
          set_led_color(0,0);
          led_status=0;
    }
}

void set_led_white_level(uint8_t level){
    uint16_t r = 0x0019;
    uint16_t gb = 0x1919;
    r = r*level;
    gb = gb*level;
    set_led_color(r,gb);
}

void set_led_blue_level(uint8_t level){
    uint16_t r = 0x0000;
    uint16_t gb = 0x0019;
    r = r*level;
    gb = gb*level;
    set_led_color(r,gb);
}