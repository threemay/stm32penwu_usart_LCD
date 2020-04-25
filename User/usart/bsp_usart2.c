
#include "bsp_usart2.h"
#include <string.h>
#include "./data_process/data_process.h"

uint8_t SendBuff2[SENDBUFF2_SIZE];
uint8_t Usart2_Rx_Buf[USART2_RX_BUFF_SIZE];

static void USART2_DMA_Rx_Config(void);
static void USART2_DMA_Tx_Config(void);
void USART2_Receive_DataPack(void);

static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* 嵌套向量中断控制器组选择 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
  /* 配置USART为中断源 */
  NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART2_IRQ;
  /* 抢断优先级*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  /* 子优先级 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  /* 使能中断 */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* 初始化配置NVIC */
  NVIC_Init(&NVIC_InitStructure);
}


/**
  * @brief  USART GPIO 配置,工作参数配置
  * @param  无
  * @retval 无
  */
void USART2_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开串口GPIO的时钟
	DEBUG_USART2_GPIO_APBxClkCmd(DEBUG_USART2_GPIO_CLK, ENABLE);
	
	// 打开串口外设的时钟
	DEBUG_USART2_APBxClkCmd(DEBUG_USART2_CLK, ENABLE);

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART2_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART2_TX_GPIO_PORT, &GPIO_InitStructure);

  // 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART2_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART2_RX_GPIO_PORT, &GPIO_InitStructure);
	
	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = DEBUG_USART2_BAUDRATE;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(DEBUG_USART2, &USART_InitStructure);	
	// 串口中断优先级配置
	NVIC_Configuration();
	
	
	USART_ITConfig(DEBUG_USART2, USART_IT_IDLE, ENABLE);  
	USART_DMACmd(DEBUG_USART2, USART_DMAReq_Rx, ENABLE); 
	USART2_DMA_Rx_Config();
	USART2_DMA_Tx_Config();
	// 使能串口
	USART_Cmd(DEBUG_USART2, ENABLE);	    
}


void USART2_DMA_Tx_Config(void)
{
		DMA_InitTypeDef DMA_InitStructure;
	
		// 开启DMA时钟
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
		// 设置DMA源地址：串口数据寄存器地址*/
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART2_DR_ADDRESS;
		// 内存地址(要传输的变量的指针)
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)SendBuff2;
		// 方向：从内存到外设	
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		// 传输大小	
		DMA_InitStructure.DMA_BufferSize = SENDBUFF2_SIZE;
		// 外设地址不增	    
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		// 内存地址自增
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		// 外设数据单位	
		DMA_InitStructure.DMA_PeripheralDataSize = 
	  DMA_PeripheralDataSize_Byte;
		// 内存数据单位
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	 
		// DMA模式，一次或者循环模式
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
		//DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	
		// 优先级：中	
		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 
		// 禁止内存到内存的传输
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		// 配置DMA通道		   
		DMA_Init(USART2_TX_DMA_CHANNEL, &DMA_InitStructure);		
		// 使能DMA
		DMA_Cmd (USART2_TX_DMA_CHANNEL,ENABLE);
}

static void USART2_DMA_Rx_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	// 开启DMA时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	// 设置DMA源地址：串口数据寄存器地址*/
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)USART2_DR_ADDRESS;
	// 内存地址(要传输的变量的指针)
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Usart2_Rx_Buf;
	// 方向：从内存到外设	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	// 传输大小	
	DMA_InitStructure.DMA_BufferSize = USART2_RX_BUFF_SIZE;
	// 外设地址不增	    
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	// 内存地址自增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	// 外设数据单位	
	DMA_InitStructure.DMA_PeripheralDataSize = 
	DMA_PeripheralDataSize_Byte;
	// 内存数据单位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	 
	// DMA模式，一次或者循环模式
	//DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;	
	// 优先级：中	
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; 
	// 禁止内存到内存的传输
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	// 配置DMA通道		   
	DMA_Init(USART2_RX_DMA_CHANNEL, &DMA_InitStructure);		
	// 清除DMA所有标志
	DMA_ClearFlag(DMA1_FLAG_TC5);
	DMA_ITConfig(USART2_RX_DMA_CHANNEL, DMA_IT_TE, ENABLE);
	// 使能DMA
	DMA_Cmd (USART2_RX_DMA_CHANNEL,ENABLE);
}

void DEBUG_USART2_IRQHandler(void)
{
	/* 使用串口DMA */
	if(USART_GetITStatus(DEBUG_USART2,USART_IT_IDLE)!=RESET)
	{		
		/* 处理数据 */
		USART2_Receive_DataPack();
		// 清除空闲中断标志位
		USART_ReceiveData( DEBUG_USART2 );
	}	

}

void Usart2_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到USART */
	USART_SendData(pUSARTx,ch);
		
	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

void HMISendb(uint8_t k)		         //字节发送函数
{		 
	uint8_t i;
	 for(i=0;i<3;i++)
	 {
	 if(k!=0)
	 	{
			USART_SendData(USART2,k);  //发送一个字节
			while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET){};//等待发送结束
		}
	 else 
	 return ;

	 } 
}

void HMISends(char *str)
{
	unsigned int k=0;
  do 
  {
      Usart2_SendByte( DEBUG_USART2, *(str + k) );
      k++;
  } while(*(str + k)!='\0');
	
  /* 等待发送完成 */
  while(USART_GetFlagStatus(DEBUG_USART2,USART_FLAG_TC)==RESET)
  {}
	HMISendb(0xff);
}

	
void USART2_Receive_DataPack(void)
{
	/* 接收的数据长度 */
	uint32_t buff_length;
	uint8_t *pbuff = Usart2_Rx_Buf;
	
	/* 关闭DMA ，防止干扰 */
	DMA_Cmd(USART2_RX_DMA_CHANNEL, DISABLE);  /* 暂时关闭dma，数据尚未处理 */ 
	
	/* 获取接收到的数据长度 单位为字节*/
	buff_length = USART2_RX_BUFF_SIZE - DMA_GetCurrDataCounter(USART2_RX_DMA_CHANNEL);
	
	Usart2_data_process(pbuff);

	memset(Usart2_Rx_Buf,0,buff_length);
	
	/* 清DMA标志位 */
	DMA_ClearFlag( DMA1_FLAG_TC5 );          
	
	/* 重新赋值计数值，必须大于等于最大可能接收到的数据帧数目 */
	USART2_RX_DMA_CHANNEL->CNDTR = USART2_RX_BUFF_SIZE;    
  
	/* 此处应该在处理完数据再打开，如在 DataPack_Process() 打开*/
	DMA_Cmd(USART2_RX_DMA_CHANNEL, ENABLE);      

}

void USART2_send(void)
{
	int i;
	for(i=0;i<SENDBUFF2_SIZE;i++)
  {
    SendBuff2[i]	 = '1';
  }
  DMA_Cmd(DMA1_Channel7, DISABLE);
                               
  DMA1_Channel7->CNDTR =  20;   //重新设定传输数据个数  
                               
  DMA_Cmd(DMA1_Channel7, ENABLE);
		 	 
}


