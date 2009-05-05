#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define WR_ADDR 	0xa0
#define RD_ADDR 	0xa1


void set_data(struct i2c_rdwr_ioctl_data *i2c_data)
{
	char tmp[255]="sample 123456";
	
	i2c_data->msgs->addr = 0xa0; // write
	i2c_data->msgs->buf  = tmp;
	i2c_data->msgs->len  = strlen(tmp)+1;
	i2c_data->nmsgs =1;

	return;
}

int main(void)
{
	int fd;
	int res;
	int n,m;
	unsigned char buf[255],tmp;

	struct i2c_rdwr_ioctl_data i2c_data;
	
	fd = open("/dev/i2c/0",2);
	if(fd < 0)
	{
		printf("####i2c test device open fail####\n");
		return (-1);
	}
	printf("--> i2c open file device %d \n",fd);
	

	res = ioctl(fd,I2C_TENBIT,0);
	res = ioctl(fd,I2C_SLAVE_FORCE,WR_ADDR>>1);
	printf("^ ioctl I2C_SLAVE_FORCE msg NO:%d\n",res);


	buf[0]=0x00;
	for(n=1;n<31;n++)
		buf[n] = (unsigned char)n;
	
//	strcat(buf,"sample test 123",strlen("sample test 123")+1);
//	printf("write buf : [%s]\n",buf);
	
//res = write(fd,buf,strlen(buf)+1);
	res = write(fd,buf,10);
	printf("^^^ ioctl I2C_WR msg NO:%d\n",res);
	//write(fd,i2c_data.msgs->buf,strlen(i2c_data.msgs->buf)+1);

//	strcpy(buf,"");

//	printf("^^i^^ ioctl I2C_RDWR msg NO:%s\n",buf);

	for(n=0;n<3;n++)
	{
		tmp = (unsigned char)(n);
		
		res = write(fd,&tmp,1);
		res = read(fd,&buf,n+1);
		
		for(m=0;m<3;m++)
			printf("^^^^ ioctl I2C_RD msg NO:%x\n",buf[m]);
	}
	

	//res = ioctl(fd,I2C_RDWR,&i2c_data);
	//printf("^^^ ioctl I2C_RDWR msg NO:%d\n",res);
    //printf("--> i2cwrite %s\n",i2c_data.msgs->buf);
	
	close(fd);
	return(0);
}

