#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
//#include "key.h"     
//#include "exti.h"
//#include "malloc.h"
//#include "sdio_sdcard.h"     
//#include "ff.h"  
//#include "exfuns.h"    
#include "uc8088_spi.h"

//#include <string.h>

u8 which_buf = 0, send_flag = 1;
u8 Buffer[2][SPI_BUF_LEN] = {0};
u8 send_cmd[16] = "AT+MIPSEND=1,\0";
const char str_OK[]="SEND OK";

void ByteChange(register u8 *pBuf, s16 len)
{
	register s16 i;
	register u8 ucTmp;
	
	for(i = 0; i < len; i += 4)
	{
			ucTmp = pBuf[i+3];
			pBuf[i+3] = pBuf[i];
			pBuf[i] = ucTmp;
			ucTmp = pBuf[i+2];
			pBuf[i+2] = pBuf[i+1];
			pBuf[i+1] = ucTmp;
	}
}

void ML302_init()
{
	printf("AT\r\n");			
	delay_ms(10);
	printf("AT+CPIN?\r\n");		//��ѯSIM��״̬
	delay_ms(50);
	printf("AT+CSQ\r\n");			//��ѯ�ź������� С��10˵���źŲ�
	delay_ms(50);
//	printf("AT+CGDCONT=1,\"IP\",\"CMIOT\"\r\n");		//����APN
//	delay_ms(10);
	printf("AT+CGACT=1,1\r\n");		//����PDP
	delay_ms(100);
	printf("AT+MIPOPEN=1,\"TCP\",\"server.natappfree.cc\",33586\r\n");	//���ӷ�����
	delay_ms(100);
	printf("ATE0\r\n");				//�رջ���
	delay_ms(10);
	memset(USART_RX_BUF, 0, USART_REC_LEN);
	USART_RX_STA = 0;
}


//int ML302_send_result()
//{
//	u32 time_out = 1e6;
//	while(time_out && !(USART_RX_STA & 0x8000))		//�ȴ� ML302 ���ط��ͽ��
//	{	time_out--; LED1=!LED1;}
//	if(!time_out)
//		printf("***---%s---%x****", USART_RX_BUF, USART_RX_STA);
//	USART_RX_STA = 0;
//	if (NULL == strstr(USART_RX_BUF, str_OK))
//	{
//		delay_ms(1);
//		printf("-*-*-*%s---%x*-*-*", USART_RX_BUF, USART_RX_STA);
//		memset(USART_RX_BUF, 0, USART_REC_LEN);
//		return -1;
//	}
//	else
//	{
//		memset(USART_RX_BUF, 0, USART_REC_LEN);
//		return 0;
//	}
//}

void uart_send_data_2_ML302(register u8 *TX_BUF, u16 len)
{
	char num_char[4];
//	u16 t;
	sprintf(num_char, "%d", len);
	printf("%s%s\r\n", send_cmd, num_char);
	TX_BUF[len] = '\0';
	printf("%s", TX_BUF);
//	for(t=0; t<len; t++)
//	{
//		USART_SendData(USART1, *(TX_BUF + t));//�򴮿�1��������
//		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
//	}
}

//FATFS fs;													/* FatFs�ļ�ϵͳ���� */
//FIL fnew;													/* �ļ����� */
//FRESULT res_flash;                /* �ļ�������� */
//UINT fnum;            					  /* �ļ��ɹ���д���� */
////BYTE ReadBuffer[1024]={0};        /* �������� */
//BYTE WriteBuffer[] = "���д���ļ���123abc";         /* д������*/

int main(void)
{		
	u8  rp_OK, wp_OK, cnt, wp_stop_flag;
	u16 tmp, tmp1, len, pos;
	u32 wp, rp, rrp;
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200	
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	//KEY_Init();					//��ʼ������
	uc8088_init();		//8088��ʼ��
	
	printf("halt cpu\t");
	rp = uc8088_read_u32(0x1a107018);
	printf("read test = %x\r\n", rp);
	
	wp = uc8088_read_u16(Buf_addr);
	printf("wp = %x\r\n", wp);
	wp = uc8088_read_u16(Buf_addr+2);
	printf("wp = %x\r\n", wp);
	wp = 0;

	wp = uc8088_read_u32(Buf_addr);
	rp = uc8088_read_u32(Buf_addr + 4);
	printf("wp = %d,   rp = %d\r\n", wp, rp); 
	LED0 = 1;
	ML302_init();
	wp = 65536;
	rp = 0;
	rrp = 0;
	pos = 0;
	wp_stop_flag = 0;

	while(1){
		
		cnt = 0;	
		do
		{
			if(rp == rrp)
			{
				rp_OK = 1;
				break;
			}
			rp = uc8088_read_u32(Buf_addr + 4);
			if(cnt++ > 5)
			{
				rp_OK = 0;
				printf("read rp = %d is error!�� right rp = %d\r\n", rp, rrp);
				break;
			}
		}while(1);
		
		cnt = 0;
		do
		{
			wp = uc8088_read_u32(Buf_addr);
			if (wp < SPI_BUF_LEN)		
			{
				len = (rp <= wp) ? (wp - rp) : (SPI_BUF_LEN - rp + wp);
				if (len < 64){
					wp_OK = 0;
					wp_stop_flag = 1;
				}
				else 
					wp_OK = 1;
				break;
			}

			if(cnt++ > 5)
			{
				wp_OK = 0;
				printf("read wp = %d is error!\r\n", wp);
				break;
			}
		}while(1);
		
		if(rp_OK && wp_OK){
			LED1 = 1;
			tmp = 0;
			cnt = 0;
			do{
				if (rp < wp){
					tmp = uc8088_read_memory(Buf_addr + 8 + rp, Buffer[which_buf] + pos, len);
					tmp1 = rp+tmp;
					pos += tmp;
				}
				else{
					tmp = uc8088_read_memory(Buf_addr + 8 + rp, Buffer[which_buf]+ pos, SPI_BUF_LEN - rp);
					tmp1 = uc8088_read_memory(Buf_addr + 8, Buffer[which_buf]+ pos + tmp, wp);
					tmp += tmp1;
					pos += tmp;
				}
				
				if(cnt++ > 5)		//��ȡuc8088�ڴ�ʧ��
				{
						printf("read memory is error!\r\n");
						break;
				}
			}while(!tmp);
			if(tmp)
			{
				cnt = 0;  	rp_OK=0;		wp_OK=0;  rrp = 65536;
				do{
						uc8088_write_u32(Buf_addr + 4, tmp1);
						rrp = uc8088_read_u32(Buf_addr + 4);
						
						if(cnt++ > 5)		//�޸�дָ��ʧ��
						{
							printf("write rp = %d, not eq %d, so is error!\r\n", rrp, tmp1);
							break;
						}
				}while(rrp != tmp1);
			}
		}
		
		if(pos > 3600)
			send_flag = 1;
		
		//�յ�1KB�������ݾͷ���ML302
		if(pos > 1024 && send_flag)
		{
			send_flag = 0;
			LED0 = 0;			//����  æ, STM32 ���ܶ�ȡuc8088, �ʿ��ܶ�����
			ByteChange(Buffer[which_buf], pos);		//�ֽڷ�ת
//			cnt = 0;
//			do{
			uart_send_data_2_ML302(Buffer[which_buf], pos);
			
//				if (cnt++ > 5){
//					printf("send to ML302 error\r\n");
//					break;
//				}
//			}while(ML302_send_result());
			LED0 = 1;
			which_buf = !which_buf;
			pos = 0;
		}
		
		if(wp_stop_flag)
		{
			LED1 = 0;				//����
		}
		
	}
}


// 
// int main(void)
// {	 
//	char USART_RX_BUF1[11], USART_RX_BUF2[11];
//	u16 USART_RX_cnt2, USART_RX_cnt1;
//	u32 total,free;
//	u32 wp, rp;
//	delay_init();	    	 //��ʱ������ʼ��	  
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
//	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200	
// 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
//	KEY_Init();					//��ʼ������
//	uc8088_init();		//8088��ʼ��
//	EXTIX_Init(); //�ⲿ�жϳ�ʼ��
// 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��

//	while(SD_Init())//��ⲻ��SD��
//	{
//		printf("SD Card Error!");
//		delay_ms(200);					
//		LED0=!LED0;//DS0��˸
//	}
//	
// 	exfuns_init();							//Ϊfatfs��ر��������ڴ�				 
//  f_mount(fs[0],"0:",1); 					//����SD�� 	
//		  
//	while(exf_getfree((u8*)'0', &total, &free))	//�õ�SD������������ʣ������
//	{
//		printf("SD Card Fatfs Error!\n");
//		delay_ms(200);
//		LED0=!LED0;//DS0��˸
//	}													  			    
// 					
//	printf("SD Total Size: %d    MB\r\n", total>>10);	//��ʾSD�������� MB
//	printf("SD  Free Size: %d    MB\r\n", free>>10);	//��ʾSD��ʣ������ MB	
//	
//	printf("\r\n******���������ļ�д�����... ******\r\n");	
//	res_flash = f_open(&fnew, "0:gps_test_data\\out_test.log",FA_CREATE_ALWAYS | FA_WRITE );
//	if ( res_flash == FR_OK )
//	{
//		printf("����/����FatFs��д�����ļ�.txt�ļ��ɹ������ļ�д�����ݡ�\r\n");
//		LED0 = 1;
//		LED1 = 1;
//	}
//	else
//	{	
//		printf("������/�����ļ�ʧ�ܡ�\r\n");
//		LED0 = 0;
//		while(1)
//		{
//			delay_ms(300);
//			LED1 = ~LED1;
//		}
//	}
//	
//	printf("halt cpu\t");
//	rp = uc8088_read_u32(0x1a107018);
//	wp = uc8088_read_u32(config_enable_Addr);
//	printf("rp = %x,\t wp = %x\r\n", rp, wp);
//	
//	total = 0;
//	while(1)
//	{
//		
//		if (key0_flag)	// ��������
//		{
//			LED0 = 0;
//			LED1 = 0;
//			printf("�ļ�����ɹ�\r\n");
//			while(1){
//				delay_ms(1000);
//				LED0 = ~LED0;
//				LED1 = ~LED1;
//			}
//		}
//		
//		// ���дBUF2 �Ѿ�д��
//		if(USART_RX_cnt2 == USART_REC_LEN){
//			res_flash = f_write(&fnew, USART_RX_BUF2, USART_REC_LEN, &fnum);
//			if (res_flash==FR_OK && fnum == USART_REC_LEN){
//				LED0 = 1;
//				LED1 = 0;
//				USART_RX_cnt2 = 0;
//			}
//			else
//				printf("�����ļ�д��ʧ�ܣ�(res_flash = %d), fnum = %d\n",res_flash, fnum); 
//		}
//		else if(USART_RX_cnt1 == USART_REC_LEN){
//			res_flash = f_write(&fnew, USART_RX_BUF1, USART_REC_LEN, &fnum);
//			if (res_flash==FR_OK && fnum == USART_REC_LEN){
//				LED0 = 0;
//				LED1 = 1;
//				USART_RX_cnt1 = 0;
//			}
//			else
//				printf("�����ļ�д��ʧ�ܣ�(res_flash = %d), fnum = %d\n",res_flash, fnum); 
//		}
//	} 
//}

////�ⲿ�ж�0������� 
//void EXTI0_IRQHandler(void)
//{
//	delay_ms(10);//����
//	if(WK_UP==1)	 	 //WK_UP����
//	{				 
//		if (key0_flag == 0)
//		{
//			/* ���ٶ�д���ر��ļ� */
//			f_close(&fnew);	
//			/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
//			f_mount(NULL,"0:",1);
//		}
//		
//		LED0 = 1;
//		LED1 = 1;
//		key0_flag = 1;
//	}
//	EXTI_ClearITPendingBit(EXTI_Line0); //���LINE0�ϵ��жϱ�־λ  
//}

//void EXTI4_IRQHandler(void)
//{
//	delay_ms(10);//����
//	if(KEY0==0)	 //����KEY0
//	{
//		if (key0_flag == 0)
//		{
//			/* ���ٶ�д���ر��ļ� */
//			f_close(&fnew);	
//			/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
//			f_mount(NULL,"0:",1);
//		}
//		
//		LED0=!LED0;
//		LED1=!LED1; 
//		key0_flag = 1;
//	}		 
//	EXTI_ClearITPendingBit(EXTI_Line4);  //���LINE4�ϵ��жϱ�־λ  
//}
// 
