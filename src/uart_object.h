/**
********************************************************************************
* @file     uart_object.h
* @author 
* @version  v0.1
* @date     2014.11.20
* @brief    This file contains an uart interface for other applications
*           which has two ring buffers (storing sending data and recieving data) 
*           and actions for communicating with outsides
********************************************************************************
**/

#ifndef _UART_OBJECT_H_
#define _UART_OBJECT_H_

/************************************************************************
* Define 'UART_PRINT_DEBUG',then informations for debug can be printed
* Undefine it when needed.
************************************************************************/
#define UART_PRINT_DEBUG

/**
********************************************************************************
* @struct struct ring_buffer
* @brief  When buffer is full ,no more data can be added 
*         head --> when new data get in , head comes to new loction
*         tail --> when data get used , tail comes to new loction
*         status --> buffer status: 0--empty ,1--available ,2--full
*         When (head+1)%SERIAL_BUFFER_SIZE != tail ,buffer is not full
*         When head == tail ,buffer is empty
********************************************************************************
**/
#define SERIAL_BUFFER_SIZE 64
typedef enum
  {
    empty = 0,
    available = 1,
    full = 2,
  }UART_BUFFER_STATUS;

struct ring_buffer
{
  unsigned char buffer[SERIAL_BUFFER_SIZE];
  volatile unsigned int head;
  volatile unsigned int tail;
  volatile UART_BUFFER_STATUS status;
};


/**
********************************************************************************
* @struct object_uart
* @brief  uart interface for other applicatons
*         one object_uart variable represents one uart channel
*         channel --> uart channel 
*         rx_buffer --> pointer to a ring buffer for rx
*         tx_buffer --> pointer to a ring buffer for tx
*         debug     --> 1 = print some debug information through uart
*                       0 = print no debug information through uart
*         getchar   --> just read a character from rx_buffer ,no waiting here
*                  o_uart -- must be the object_uart value pointer itself
*         printf --> like printf(const char *format ,...)
*                  o_uart -- must be the object_uart value pointer itself
*         set_debug --> set the 'debug' value [0 or 1]
********************************************************************************
**/
typedef struct object_usart object_uart;

struct object_usart
{
  unsigned char channel;
  unsigned char debug;
  struct ring_buffer *rx_buffer;
  struct ring_buffer *tx_buffer;
  int (*getchar)(object_uart *o_uart);
  int (*printf)(object_uart *o_uart ,const char *format ,...);
  void (*set_debug)(object_uart *o_uart,unsigned char value);
};

/**
********************************************************************************
* @variable  usart1
* @brief     Global variable name for object_uart
*            Applications can use either the 'usartX' here or some other names
*            when needed.
*            These 'usartX' are not initialized by default.
********************************************************************************
**/

#define USART_GLOBAL_NAME

#ifdef USART_GLOBAL_NAME
object_uart usart1; 
#endif

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
extern int object_uart_init(object_uart * o_uart,unsigned int channel);

#endif
