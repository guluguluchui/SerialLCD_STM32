/**
********************************************************************************
* @file     ATcmd_object.h
* @author   Mars.Wu
* @version  v0.1
* @date     2014.11.24
* @brief    This file contains an ATcmd interface for other applications
*           which has some arrays of strings (each string is a command) 
*           and actions for geting/excuting ATcmd
********************************************************************************
**/

#ifndef _ATCMD_OBJECT_H_
#define _ATCMD_OBJECT_H_

/**
********************************************************************************
* @Micro  MAX_ATCMD_NUM
* @brief  Max command number.
*         Commands are stored in array ATcmd[MAX_ATCMD_NUM][6]
********************************************************************************
**/

#define MAX_ATCMD_NUM 38  

/**
********************************************************************************
* @struct ATcmd_buffer
* @brief  The received command is stored in this buffer.
*         Only one command can be stored in this buffer at one time.
********************************************************************************
**/
#define ATCMD_BUFFER_SIZE 200

typedef enum  
  {
    not_start = 0,       /* no "AT+" received yet                */
    incomplete = 1,      /* "AT+" is ok ,but ";" is not received */
    complete = 2,        /* ";" is ok ,command is complete       */
  }ATCMD_BUFFER_STATUS;  /* ATcmd_buffer status .                */

struct ATcmd_buffer
{
  unsigned char buf[ATCMD_BUFFER_SIZE];
  volatile unsigned int new_position;
  volatile ATCMD_BUFFER_STATUS status;
};   

/**
********************************************************************************
* @struct object_ATcmd
* @brief  ATcmd interface for other applicatons
*         command --> ATcmd_buffer ,store the received command.
*         get_cmd --> pointer to a function that stores one command into buffer
*         execute --> pointer to function that analysizes a command and 
*                     execute the related actions.
*         action_array --> an array links to actions (functions)
*                          Those actions are related to the command in AT_cmd[][] 
********************************************************************************
**/

/********************************************
* The action_array function type.
* args : store the parameters in one command
* arg_num : parameters' number in one command
*********************************************/
typedef void (*func_t)(unsigned char **args, int arg_num);

typedef struct object_ATcommand object_ATcmd;

struct object_ATcommand
{
  struct ATcmd_buffer command;
  int (*get_cmd)(object_ATcmd *at_cmd);
  int (*execute)(object_ATcmd *at_cmd);
  func_t action_array[MAX_ATCMD_NUM];
};

/**
********************************************************************************
* @variable  at_cmd
* @brief     Global variable name for object_ATcmd
*            Applications can use either the 'at_cmd' here or some other names
*            when needed.
*            These 'at_cmd' are not initialized by default.
********************************************************************************
**/
#define GLOBAL_ATCMD_NAME

#ifdef GLOBAL_ATCMD_NAME
object_ATcmd at_command;
#endif

/**
********************************************************************************
*  @function    int object_ATcmd_init(object_ATcmd * at_cmd);
*  @brief       Set up an ATcmd object interface.
*               Here , the 'get_cmd' 'execute' & 'action_array' are link to 
*               the real funcitons.
*               ATcmd_buffer is also initialized.
*  @return      return 0  --> succeed
*               return -1 --> fail
********************************************************************************
**/
extern int object_ATcmd_init(object_ATcmd * at_cmd);

#endif
