/**
********************************************************************************
* @file     uart_object.c
* @author 
* @version  v0.1
* @date     2014.11.20
* @brief    This file contains all functions that support object_uart interface
*           Interrupts get used to transport data between buffers and usartX
********************************************************************************
**/

#include <stdio.h>
#include <stdarg.h>
#include "uart_object.h"
#include <stm32f10x_conf.h>
#include "stm32f10x_usart.h"

/**
********************************************************************************
* @variable rx_buffer_isr1
*           tx_buffer_isr1
*           usart1
* @brief    Each Uart Channel has two own ring buffers.
*           rx_buffer_isrX is Uart Channe X rx_buffer.
*           tx_buffer_isrX is Uart Channe X tx_buffer.
*           These ring buffers variables can be used by the ISRs. 
*           and related to the Object_uart variable.
*           Variable usartX is global for device uartX ,default not initialized.
*           Applications can use either the 'usartX' or other names when needed.
********************************************************************************
**/
static struct ring_buffer rx_buffer_isr1 = {{'\0'},0,0,empty};
static struct ring_buffer tx_buffer_isr1 = {{'\0'},0,0,empty};

/**
********************************************************************************
* @funtion  store_char(unsigned char c, struct ring_buffer *buffer)
* @brief    Transport one character to rx_buffer_isrX
*           When buffer is full , this can not be done
*           When buffer is not full ,head always comes to the new empty loction
*           This function is used by the Uart ISR
* @return   return  0 --> succeed
*           return -1 --> fail 
********************************************************************************
**/
static int store_char(unsigned char c, struct ring_buffer *buffer)
{
  unsigned int i = (unsigned int)(buffer->head + 1) % SERIAL_BUFFER_SIZE;

  /*******************************************************************
  If we should be storing the received character into the location
  just before the tail (meaning that the head would advance to the
  current location of the tail), we're about to overflow the buffer
  and so we don't add this character or advance the head.
  The head always comes to the next empty loction
  *******************************************************************/
  if(buffer->status != full)
    {
      buffer->buffer[buffer->head] = c;  /* add new character */
      buffer->head = i;                  /* head comes to next empty loction */
      i = (i + 1) % SERIAL_BUFFER_SIZE;  /* ready to see if it's full now */
      buffer->status = (i == buffer->tail)?full:available; /* update status */	  
      return 0;
  }
  return -1;
}


/**
********************************************************************************
* @funtion  int Serial1Read(object_uart *o_uart)
* @brief    read one character from the rx_buffer and return the chareacter
* @return   return  c --> character
*           return -1 --> fail
********************************************************************************
**/
static int Serial1Read(object_uart *o_uart)
{
  unsigned char c;
  unsigned int tmp_tail,tmp_head; 

  if(o_uart->rx_buffer->status != empty)
    {
      /**************************************************
       tmp_tail : The next loction to be read
       tmp_head : The head loction
      **************************************************/
      tmp_tail = (unsigned int)(o_uart->rx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
      tmp_head = o_uart->rx_buffer->head;

      c = o_uart->rx_buffer->buffer[o_uart->rx_buffer->tail]; /*get character */ 
      o_uart->rx_buffer->buffer[o_uart->rx_buffer->tail] = '\0';
      o_uart->rx_buffer->tail = tmp_tail;  /*tail comes to the next loction */

      /*update the rx_buffer status*/
      o_uart->rx_buffer->status = (tmp_tail == tmp_head)?empty:available;

      return c;
    }
  return -1;
}

/**
********************************************************************************
* @funtion  int Serial1Write(object_uart *o_uart ,char c)
* @brief    add one character to the tx_buffer
* @return   return  0 --> secceed
********************************************************************************
**/
static int Serial1Write(object_uart *o_uart, char c)
{
  unsigned int i;
  i = (unsigned int)(o_uart->tx_buffer->head + 1) % SERIAL_BUFFER_SIZE;

  /****************************************************************************
   Character can be send by uart hardware, so this buffer won't be always full
   Wait until buffer is available 
  ****************************************************************************/
  while (o_uart->tx_buffer->status == full); 

  o_uart->tx_buffer->buffer[o_uart->tx_buffer->head] = c; /*add new character*/
  o_uart->tx_buffer->head = i;  /* head comes to next empty loction */
  USART_ITConfig(USART1, USART_IT_TXE, ENABLE);  /*enable the TXE interrupt*/

  i = (i + 1) % SERIAL_BUFFER_SIZE;  /* ready to see if it's full now */
  /* update status */	  
  o_uart->tx_buffer->status = (i == o_uart->tx_buffer->tail)?full:available;
  return 0;
}

/**
********************************************************************************
* @funtion  void Serial1Print(char * dat)
* @brief    send a string to outsides
********************************************************************************
**/
static void SerialPrint(object_uart *o_uart,char * dat)
{
  char * p = dat;
  while(*p != '\0')
    {
      Serial1Write(o_uart,*p);
      p++;
    }
}


/**
********************************************************************************
* @funtion  int print_from_uart(object_uart *o_uart,const char* format, ...)
* @brief    printf() to outsides through Uart
* @return   return ret --> numbers for characters have been sent
*           return -1  --> fail
********************************************************************************
**/
static int print_from_uart(object_uart *o_uart,const char* format, ...)
{
  int ret;
  va_list ap;

  va_start(ap, format);

  static char buf[SERIAL_BUFFER_SIZE];

  /**************************************************************
  * Print to the local buffer
  * This fucntion needs to be rewrite ,as it takes too much time
  **************************************************************/
  ret = vsnprintf (buf, sizeof(buf), format, ap);
  if (ret > 0)
    {
      /*Transfer the buffer to the device*/
      SerialPrint(o_uart ,buf);
    }

  va_end (ap);
  return ret;
}

/**
********************************************************************************
* @funtion  void set_debug_uart(object_uart *o_uart,unsigned char value)
* @brief    value is 0 or 1
*           0 --> no debug information can be sent through usart
*           1 --> some debug information can be sent through usart
********************************************************************************
**/
static void set_debug_uart(object_uart *o_uart,unsigned char value)
{
  o_uart->debug = value;
  if(o_uart->debug == 1)
    {
      o_uart->printf(o_uart,"debug flag is %d\n",o_uart->debug);
    }
}


/**
********************************************************************************
* @funtion  void USART1_IRQHandler(void)
* @brief    USART1 ISR
*           transport a recieved character to rx_buffer_isr1 or
*           send a character from tx_buffer_isr1 to outsides
********************************************************************************
**/
void USART1_IRQHandler(void)
{
  char c;
  FlagStatus RXNE,TXE;
	
  /*Checks whether the specified USART flag is set or not*/
  RXNE = USART_GetFlagStatus(USART1,USART_FLAG_RXNE); 
  if (RXNE == SET)  /*Receive data register not empty*/
    {
      c = USART_ReceiveData(USART1);  /*read a character*/
      store_char(c, &rx_buffer_isr1); /*add a character to rx_buffer_isr1 */
    }

  /*Checks whether the specified USART flag is set or not*/
  TXE = USART_GetFlagStatus(USART1,USART_FLAG_TXE);
  if (TXE == SET)  /*Transmit data register empty flag*/
    {
      if (tx_buffer_isr1.status == empty) /*buffer is empty*/
	{
	  USART_ITConfig(USART1, USART_IT_TXE, DISABLE); /*disable TXE interrupter*/
	}
      else
	{
	  /*get a character to be sent*/
	  c = tx_buffer_isr1.buffer[tx_buffer_isr1.tail]; 
	  /*update buffer.tail*/
	  tx_buffer_isr1.tail = (tx_buffer_isr1.tail + 1) % SERIAL_BUFFER_SIZE;
	  USART_SendData(USART1,c);

	  /*update buffer status */
	  tx_buffer_isr1.status = 
	    (tx_buffer_isr1.tail == tx_buffer_isr1.head)?empty:available;

	}
    }	
}


/**
********************************************************************************
*  @function    object_uart_init(object_uart *,usigned int channel)
*  @brief       Set up an uart struct object with it's channel
*               Default BPS         -- 115200
*                       Word_length -- 8 bits
*                       Stop_Bits   -- 1 bit
*                       Parity      -- none
*                       HW Flow     -- none
*  @return      return 0  --> succeed
*               return -1 --> fail
********************************************************************************
**/
int object_uart_init(object_uart * o_uart, unsigned int channel)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  switch(channel)	
    {
    case 1:
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
			     RCC_APB2Periph_AFIO | 
			     RCC_APB2Periph_USART1,
			     ENABLE); 
	
	
      GPIO_StructInit(&GPIO_InitStructure);
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
      GPIO_Init(GPIOA , &GPIO_InitStructure);
 
       
      GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
      GPIO_Init(GPIOA, &GPIO_InitStructure);
	
      USART_InitStructure.USART_BaudRate = 115200;
      USART_InitStructure.USART_WordLength = USART_WordLength_8b;
      USART_InitStructure.USART_StopBits = USART_StopBits_1;
      USART_InitStructure.USART_Parity = USART_Parity_No;
      USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
      USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
      USART_Init(USART1, &USART_InitStructure);
	
      USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
      USART_Cmd(USART1, ENABLE);
	
      NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
      /* Enable the USARTy Interrupt */
      NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
      NVIC_Init(&NVIC_InitStructure);


      break;
    case 2:
    case 3:
    case 4:
    case 5:
    default:
      return -1;
    }

  o_uart->channel = channel;
  o_uart->debug = 0;

  o_uart->rx_buffer = &rx_buffer_isr1;
  o_uart->rx_buffer->head = 0;
  o_uart->rx_buffer->tail = 0;
  o_uart->rx_buffer->status = empty;

  o_uart->tx_buffer = &tx_buffer_isr1;
  o_uart->tx_buffer->head = 0;
  o_uart->tx_buffer->tail = 0;
  o_uart->tx_buffer->status = empty;

  o_uart->getchar = Serial1Read; 
  o_uart->printf = print_from_uart;
  o_uart->set_debug = set_debug_uart;
  return 0;
}
