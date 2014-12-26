/**
********************************************************************************
* @file     ATcmd_object.c
* @author   Mars.Wu
* @version  v0.1
* @date     2014.11.25
* @brief    This file contains all functions that support object_ATcmd interface
*           Note: some functions need initialized global 'usart1' and 'lcd'.
********************************************************************************
**/

#include <string.h>        /* strncmp() & strtok() */
#include "ATcmd_object.h"  /* object_ATcmd interface */
#include "uart_object.h"   /* object_uart interface & global 'usart1' */
#include "lcd_object.h"  /* object_lcd interface & global 'lcd' */

/**
********************************************************************************
* @Array  ATcmd[]
* @brief  AT commands array.
*         Only commands in this array can be recongnized and executed.
*         One complete command includes :
*           (1)start      --  "AT+"
*           (2)command    --  2 characters followed by "AT+"
*           (3)parameter  --  followed by "=" , separated by ","
*                             Note: parameter is optional.
*           (4)end        --  ";"
*         The characters '\0' '\n' are ignored after the "AT+".
*         For example :
*            1. AT+  cs;  [same with "AT+cs"]   
*            2. AT+ fs = 200 , 23,   123  ; [same with "AT+fs=200,23,123;"]   
*         Note : 4 commands below with [*] in each content are special.
*                These 4 commands are not listed in the UserMannual.
********************************************************************************
**/
static const unsigned char ATcmd[MAX_ATCMD_NUM][6] =
 {
    "AT+SD", /* [*] Set Device */
    "AT+II", /* [*] Init       */

    "AT+GX", /* Get the width of the screen in the current orientation  */
    "AT+GY", /* Get the height of the screen in teh current orientation */
    "AT+DO", /* Device on        */
    "AT+DF", /* Device off       */
    "AT+SC", /* Set Contrast     */
    "AT+SB", /* Set Brightness   */
    "AT+SP", /* Set Display Page */
    "AT+WP", /* Write Page       */

    "AT+cs", /* Clear Screen          */
    "AT+fs", /* Fill Screen with a specified color */
    "AT+sf", /* Set Front Color       */
    "AT+gf", /* Get Front Color       */
    "AT+sb", /* Set Back Color        */
    "AT+gb", /* Get Back Color        */
    "AT+dp", /* Draw Point            */
    "AT+dl", /* Draw line             */
    "AT+dr", /* Draw Rectangele       */
    "AT+dc", /* Draw Circle           */
    "AT+dR", /* Draw Round Rectangele */
    "AT+fr", /* Fill Rectangle        */
    "AT+fc", /* Fill Circle           */
    "AT+fR", /* Fill Round Rectangle  */
    "AT+ps", /* Print String          */
    "AT+pi", /* Print Integer         */
    "AT+pf", /* Print Float           */
    "AT+sF", /* Set Font              */
    "AT+gF", /* Get Font              */
    "AT+gX", /* Get Font Xsize        */
    "AT+gY", /* Get Font Ysize        */
    "AT+dB", /* Draw Bitmap           */

    "AT+DT", /* Data */
    "AT+ED", /* End  */
    "AT+EW", /* [*] Eeprom Write */
    "AT+FW", /* [*] Flash Write  */

    "AT+gt", /* Get TouchXY */
    "AT+DB"  /* set debug flag */
  };




/**
********************************************************************************
* @function  void add_char_to_cmd_buf(object_ATcmd *at_cmd,unsigned char c)
* @brief     Add a character to the ATcmd_buffer.
*            Currently ,the ATcmd must not be longer than ATCMD_BUFFER_SIZE.
* @param     at_cmd --> must be the object_ATcmd variable itself
*            c      --> a character to be add to the command buffer
* @return    0  --> succeed
*            -1 --> fail (wrong or incomplete command)
********************************************************************************
**/
static void add_char_to_cmd_buf(object_ATcmd *at_cmd,int c)
{
  at_cmd->command.buf[at_cmd->command.new_position] = (unsigned char)c;
  at_cmd->command.new_position++;
}

/**
********************************************************************************
* @function  void clean_cmd_buf(object_ATcmd *at_cmd)
* @brief     Write the command buffer with '\0'.
*            Set the command buffer 'new_position' to 0.
*            Set the command status to 'not_start'.
*            After this function ,new command can be stored.
********************************************************************************
**/
static void clean_cmd_buf(object_ATcmd *at_cmd)
{
  unsigned int count;
  for(count = 0;count < at_cmd->command.new_position ; count++)
    {
      at_cmd->command.buf[count] = '\0';
    }
  at_cmd->command.new_position = 0;
  at_cmd->command.status = not_start;
}


/**
********************************************************************************
* @function  int get_a_cmd(object_ATcmd *at_cmd)
* @brief     Get a command from the USART1 and store it in command buffer.
*            This need an initilized global object_usart1 variable 'usart1'.        
*            One command starts with "AT+" and  ends with ';'.
*            Between the start and the end,
*            the characters '\0' '\n' are ignored.
*            Only one command can be in the command buffer at one time.
* @return    0  --> succeed
*            -1 --> fail (wrong or incomplete command)
********************************************************************************
**/
static int get_a_cmd(object_ATcmd *at_cmd)
{
  int c;
  
  /**************************************************
   * command status :
   * not_start --> incomplete --> complete 
   **************************************************/
  switch(at_cmd->command.status)
    {
    case not_start : /* have not gotten "AT+" now */
      c = usart1.getchar(&usart1);   /*get a character from usart1*/
     
      if( c != -1)   /*succeedd to get a character from usart1*/   
	{
	  switch((unsigned char)c)  /*check the "AT+" */     
	    {
	    case 'A':         
	      add_char_to_cmd_buf(at_cmd,c);      /* get 'A' */
	      break;
	    case 'T':
	      if(at_cmd->command.buf[0] == 'A')
		{
		  add_char_to_cmd_buf(at_cmd,c);  /* get "AT" */
		}
	      else
		{
		  clean_cmd_buf(at_cmd);   /* no "AT" , clean the 'A'*/
		  return -1;
		}
	      break;
	    case '+':
	      if(at_cmd->command.buf[1] == 'T')
		{
		  add_char_to_cmd_buf(at_cmd,c);  /* get "AT+" */
		  /* update command status*/
		  at_cmd->command.status = incomplete; 
		}
	      else
		{
		  clean_cmd_buf(at_cmd);  /* no "AT+" , clean the "AT" */
		  return -1;
		}
	      break;
	    default :
	      clean_cmd_buf(at_cmd); /* no "A" "T" "+" , clean the buffer */
	      return -1;
	    }
	}
      else  /*fail to get a character from usart1*/
	{
	  return -1;
	}
      break;
    case incomplete :
      c = usart1.getchar(&usart1);   /*get a character from usart1*/

      if( c != -1)   /*succeedd to get a character from usart1*/   
	{
	  /**************************************************
	   * As "AT+" has started , the following cmd ignores
	   * characters '\0''\n' ,and ends with ';'
	   **************************************************/
	  if( (unsigned char)c != '\0' && (unsigned char)c != '\n')
	    {
	      add_char_to_cmd_buf(at_cmd,c);
	      if( (unsigned char)c == ';')
		{
		  /* update command status*/
		  at_cmd->command.status = complete;
		  if(usart1.debug == 1)
		    {
		      usart1.printf(&usart1,"Your Inputs:");
		      usart1.printf(&usart1,"%s\n",at_cmd->command.buf);
		    }
		}
	    }
	}
      else
	{
	  return -1;
	}
      break;
    case complete :
    default:
      break;
    }
  return 0;
}

/**
********************************************************************************
* @function  int get_cmd_index(object_ATcmd *at_cmd)
* @brief     Get the command index in the array ATcmd[MAX_ATCMD_NUM][6]
*            This index can be related to object_ATcmd action array index.
* @return    count  --> the command index in ATcmd[MAX_ATCMD_NUM][6]
*            -1 --> fail
********************************************************************************
**/
static int get_cmd_index(object_ATcmd *at_cmd)
{
  int count;
  for(count = 0; count < MAX_ATCMD_NUM ;count++)
    {
      /*check the first 5 chararcters*/
      if( strncmp((const char *)at_cmd->command.buf ,
		  (const char *)&ATcmd[count][0] ,5) == 0)
	{
	  return count;
	}
    }
  return -1;
}

/**
********************************************************************************
* @function  int get_cmd_param(object_ATcmd *at_cmd,unsigned char **args)
* @brief     Get the command paramters.
*            '=' starts the first parameter.
* @return    count  --> the parameter numbers [0-6]
*            -1 --> fail
********************************************************************************
**/
static int get_cmd_param(object_ATcmd *at_cmd,char **args)
{
  const char *delim1 = ",";
  const char *delim2 = ";";
  unsigned char *p_param;
  int count;

  if(at_cmd->command.buf[5] == ';')  /* no parameter */
    {
      return 0;
    }

  if(at_cmd->command.buf[5] != '=')  /* incorrect parameter start*/
    {
      return -1;
    }

  /**************************************************
   * at_cmd->command.buf[5] is '='
   * Parameters start at at_cmd->command.buf[6]
   **************************************************/
  p_param = &at_cmd->command.buf[6]; 
  count = 0;

  /* Search ',' to get the divided parameter */
  *(args+count) = strtok( p_param, delim1);

  while(*(args+count) != NULL)
    {
      count++;
      *(args+count) = strtok( NULL, delim1);
    }

  /**************************************************
   * Delate the ';' in the last parameter
   * The last parameter is *(args+count-1)
   * The *(args+count) is always NULL here
   **************************************************/
  *(args+count-1) = strtok( *(args+count-1) , delim2);

  return count;
}

/**
********************************************************************************
* @function  int execute(object_ATcmd *at_cmd)
* @brief     Check the command and execute the related actions
*            When one command has been executed ,another one can be recieved.
*            An initilized global object_usart1 variable 'usart1' needed.  
* @return    0  --> succeed
*            -1 --> fail
********************************************************************************
**/
static int execute_a_cmd(object_ATcmd *at_cmd)
{
  /**************************************************
   * 
   **************************************************/
  int ATcmd_index;
  int arg_num;
  char *args[6];

  if(at_cmd->command.status == complete)
    {
      ATcmd_index = get_cmd_index(at_cmd);  /* get the cmd index in ATcmd[] */
      if(ATcmd_index == -1) /* command not found in the ATcmd table */
	{
	  if(usart1.debug == 1)
	    {
	      usart1.printf(&usart1,"Command does not exist.\n");
	    }
	  clean_cmd_buf(at_cmd);  
	  return -1;
	}

      arg_num = get_cmd_param(at_cmd ,args);   /* get the cmd parameter */
      if(arg_num == -1) /* fail to get command parameter*/
	{
	  if(usart1.debug == 1)
	    {
	      usart1.printf(&usart1,"Parameters error.\n");
	    }
	  clean_cmd_buf(at_cmd);  
	  return -1;
	}
      /* execute the action */
      at_cmd->action_array[ATcmd_index]((unsigned char**)args ,arg_num);

      /**************************************************
       * Clean the command buffer 
       * After this function , new command can be read
       **************************************************/
      clean_cmd_buf(at_cmd);  
      return 0;
    }
  return -1;
}

/**
********************************************************************************
*  @function    void SetDebug(unsigned char **args, int arg_num)
*  @brief       Get debug flag parameter and call the usart1.set_debug()
*               Need a global object_uart variable 'usart1'
********************************************************************************
**/
static void SetDebug(unsigned char **args, int arg_num)
{
  unsigned char flag;
  if(arg_num != 1)
    {
      if(usart1.debug == 1)
	{
	  usart1.printf(&usart1,"Parameter error.\n");
	}
      return;
    }

  flag = **args;

  switch(flag)
    {
    case '0':
      usart1.set_debug(&usart1,0);
      break;
    case '1':
      usart1.set_debug(&usart1,1);
      break;
    default :
      if(usart1.debug == 1)
	{
	  usart1.printf(&usart1,"Parameter is not '0' or '1'.\n");
	}
      break;
    }
}

/**
********************************************************************************
*  @function    int action_test(unsigned char **args, int arg_num)
*  @brief       print the parameters in one command.
*               action_array functions can related to this function. 
********************************************************************************
**/
static void action_idle(unsigned char **args, int arg_num)
{
  int count;
  if(usart1.debug == 1)
    {
      usart1.printf(&usart1,"This is object_ATcmd interface action_idle.\n");
      if(arg_num == 0)
	{
	  usart1.printf(&usart1,"No parameter. \n");
	}
      for(count = 0; count < arg_num; count++)  
	{
	  usart1.printf(&usart1,"param %d = %s\n",count+1,*(args+count));
	}
    }
}

/**
********************************************************************************
*  @function    int object_ATcmd_init(object_ATcmd * at_cmd);
*  @brief       Set up an ATcmd object interface.
*               Here , the 'get_cmd' 'execute' & 'action_array' are link to 
*               the real funcitons.
*               ATcmd_buffer is also initialized.
*               Note : the global object_lcd 'lcd' must be initialized before
*                this function.
*  @return      return 0  --> succeed
*               return -1 --> fail
********************************************************************************
**/

int object_ATcmd_init(object_ATcmd * at_cmd)
{
  unsigned int count;

  /* Initialize command buffer and status */
  for(count = 0; count < ATCMD_BUFFER_SIZE; count++)
    {
      at_cmd->command.buf[count] = '\0';
    }
  at_cmd->command.new_position = 0;
  at_cmd->command.status = not_start;

  /* Link actions to real functions */
  at_cmd->get_cmd = get_a_cmd;
  at_cmd->execute = execute_a_cmd; 

  count = 0;
  /* object_lcd interface functions */
  at_cmd->action_array[count++] = global_lcd.set_device;
  at_cmd->action_array[count++] = global_lcd.lcd_init;
  at_cmd->action_array[count++] = global_lcd.get_lcd_Xsize;
  at_cmd->action_array[count++] = global_lcd.get_lcd_Ysize;
  at_cmd->action_array[count++] = global_lcd.device_on;
  at_cmd->action_array[count++] = global_lcd.device_off;
  at_cmd->action_array[count++] = global_lcd.set_contrast;
  at_cmd->action_array[count++] = global_lcd.set_brightness;
  at_cmd->action_array[count++] = global_lcd.set_display_page;
  at_cmd->action_array[count++] = global_lcd.write_page;
  at_cmd->action_array[count++] = global_lcd.clear_screen;
  at_cmd->action_array[count++] = global_lcd.fill_screen;
  at_cmd->action_array[count++] = global_lcd.set_front_color;
  at_cmd->action_array[count++] = global_lcd.get_front_color;
  at_cmd->action_array[count++] = global_lcd.set_back_color;
  at_cmd->action_array[count++] = global_lcd.get_back_color;
  at_cmd->action_array[count++] = global_lcd.draw_point;
  at_cmd->action_array[count++] = global_lcd.draw_line;
  at_cmd->action_array[count++] = global_lcd.draw_rectangle;
  at_cmd->action_array[count++] = global_lcd.draw_cricle;
  at_cmd->action_array[count++] = global_lcd.d_round_rectangle;
  at_cmd->action_array[count++] = global_lcd.fill_rectangle;
  at_cmd->action_array[count++] = global_lcd.fill_cricle;
  at_cmd->action_array[count++] = global_lcd.f_round_rectangle;
  at_cmd->action_array[count++] = global_lcd.print_string;
  at_cmd->action_array[count++] = global_lcd.print_integer;
  at_cmd->action_array[count++] = global_lcd.print_float;
  at_cmd->action_array[count++] = global_lcd.set_font;
  at_cmd->action_array[count++] = global_lcd.get_font;
  at_cmd->action_array[count++] = global_lcd.get_font_Xsize;
  at_cmd->action_array[count++] = global_lcd.get_font_Ysize;
  at_cmd->action_array[count++] = global_lcd.draw_bitmap;
    /*other functions*/
  while(count < MAX_ATCMD_NUM - 1)
    {
      at_cmd->action_array[count++] = action_idle;
    }
  at_cmd->action_array[MAX_ATCMD_NUM -1] = SetDebug;
  return 0;
}
