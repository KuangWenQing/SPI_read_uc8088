#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "key.h"     
#include "exti.h"
#include "malloc.h"
#include "sdio_sdcard.h"     
#include "ff.h"  
#include "exfuns.h"    

 
/************************************************
�����ܣ��洢8088���ڴ�ӡ�����ݵ�SD��
************************************************/
//FATFS fs;													/* FatFs�ļ�ϵͳ���� */
FIL fnew;													/* �ļ����� */
FRESULT res_flash;                /* �ļ�������� */
UINT fnum;            					  /* �ļ��ɹ���д���� */
//BYTE ReadBuffer[1024]={0};        /* �������� */
BYTE WriteBuffer[] = "���д���ļ���123abc";         /* д������*/

 int main(void)
 {	 
	u32 total,free;

	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200	
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	EXTIX_Init(); //�ⲿ�жϳ�ʼ��
 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��

	while(SD_Init())//��ⲻ��SD��
	{
		printf("SD Card Error!");
		delay_ms(200);					
		LED0=!LED0;//DS0��˸
	}
	
 	exfuns_init();							//Ϊfatfs��ر��������ڴ�				 
  f_mount(fs[0],"0:",1); 					//����SD�� 	
		  
	while(exf_getfree((u8*)'0', &total, &free))	//�õ�SD������������ʣ������
	{
		printf("SD Card Fatfs Error!\n");
		delay_ms(200);
		LED0=!LED0;//DS0��˸
	}													  			    
 					
	printf("SD Total Size: %d    MB\r\n", total>>10);	//��ʾSD�������� MB
	printf("SD  Free Size: %d    MB\r\n", free>>10);	//��ʾSD��ʣ������ MB	
	
	printf("\r\n******���������ļ�д�����... ******\r\n");	
	res_flash = f_open(&fnew, "0:gps_test_data\\out_test.log",FA_CREATE_ALWAYS | FA_WRITE );
	if ( res_flash == FR_OK )
	{
		printf("����/����FatFs��д�����ļ�.txt�ļ��ɹ������ļ�д�����ݡ�\r\n");
		LED0 = 1;
		LED1 = 1;
    /* ��ָ���洢������д�뵽�ļ��� */
//		res_flash=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
//    if(res_flash==FR_OK)
//    {
//     printf("���ļ�д��ɹ���д���ֽ����ݣ�%d\n",fnum);
//      printf("�����ļ�д�������Ϊ��\r\n%s\r\n",WriteBuffer);
//		}
//    else
//    {
//      printf("�����ļ�д��ʧ�ܣ�(%d)\n",res_flash);
//    }    
//		/* ���ٶ�д���ر��ļ� */
//    f_close(&fnew);
	}
	else
	{	
		printf("������/�����ļ�ʧ�ܡ�\r\n");
		LED0 = 0;
		while(1)
		{
			delay_ms(300);
			LED1 = ~LED1;
		}
	}
	
/*------------------- �ļ�ϵͳ���ԣ������� ------------------------------------*/
//	printf("****** ���������ļ���ȡ����... ******\r\n");
//	res_flash = f_open(&fnew, "0:gps_test_data\\FatFs��д�����ļ�.txt", FA_OPEN_EXISTING | FA_READ); 	 
//	if(res_flash == FR_OK)
//	{
//		printf("�����ļ��ɹ���\r\n");
//		res_flash = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum); 
//    if(res_flash==FR_OK)
//    {
//      printf("���ļ���ȡ�ɹ�,�����ֽ����ݣ�%d\r\n",fnum);
//      printf("����ȡ�õ��ļ�����Ϊ��\r\n%s \r\n", ReadBuffer);	
//    }
//    else
//    {
//      printf("�����ļ���ȡʧ�ܣ�(%d)\n",res_flash);
//    }		
//	}
//	else
//	{
//		printf("�������ļ�ʧ�ܡ�\r\n");
//	}
	
	total = 0;
	while(1)
	{
		
		if (key0_flag)	// ��������
		{
			/* ���ٶ�д���ر��ļ� */
			f_close(&fnew);	
			/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
			f_mount(NULL,"0:",1);
			LED0 = 0;
			LED1 = 0;
			printf("�ļ�����ɹ�\r\n");
			while(1){
				delay_ms(1000);
				LED0 = ~LED0;
				LED1 = ~LED1;
			}
		}
		if (write_slow_flag)	// BUF�е�����û���ü�д��SD��
		{
			/* ���ٶ�д���ر��ļ� */
			f_close(&fnew);	
			/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
			f_mount(NULL,"0:",1);
			LED0 = 1;
			LED1 = 1;
			printf("��ȡBUF ̫��\r\n");
			while(1){
			}
		}
		// ���������дBUF1 �� ��USART_RX_cnt2 �Ѿ�д��
		if((witch_BUF == 1) && (USART_RX_cnt2 == USART_REC_LEN)){
			res_flash=f_write(&fnew, USART_RX_BUF2, USART_REC_LEN, &fnum);
			if (fnum == USART_REC_LEN){
				USART_RX_cnt2 = 0;
				LED0 = 1;
				LED1 = 0;
				//printf("%c", USART_RX_BUF2[511]);
			}
			else
				printf("�����ļ�д��ʧ�ܣ�(%d)\n",res_flash); 
		}
		else if(witch_BUF == 2 && USART_RX_cnt1 == USART_REC_LEN){
			res_flash=f_write(&fnew, USART_RX_BUF1, USART_REC_LEN, &fnum);
			if (fnum == USART_REC_LEN){
				USART_RX_cnt1 = 0;
				LED0 = 0;
				LED1 = 1;
				//printf("%s", USART_RX_BUF1);
			}
			else
				printf("�����ļ�д��ʧ�ܣ�(%d)\n",res_flash); 
		}
	} 
}


