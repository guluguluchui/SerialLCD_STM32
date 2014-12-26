/**
********************************************************************************
* @file     lcd_object.h
* @author   Mars.Wu
* @version  v0.1
* @date     2014.11.27
* @brief    This file contains a LCD interface for other applications
*           which has some basic functions to control LCD
********************************************************************************
**/

#ifndef _LCD_OBJECT_H_
#define _LCD_OBJECT_H_

/**
********************************************************************************
* @micro  
* @brief  Micros for different LCD's controller or moudle
********************************************************************************
**/
#define LCD_DEBUG

/**
********************************************************************************
* @struct object_lcd
* @brief  LCD interface for other applicatons
*         Each action (function) in object_lcd has parameters 'args' & 'arg_num'.
*         So other applicaitons can easily link to these functions.
*         'args' contains the real parameters,
*         'arg_num' is the parameters' number ,from 0 to 6.
*         Some functions do not need parameters ,the arg_num can be 0.
*         Some functions return values. The return value can be directly sent
*         to UART ,or stored in memery or rewritten to the 'args' when needed.
********************************************************************************
**/

typedef struct object_LCD object_lcd;

struct object_LCD
{
  unsigned int index;
  void (*set_device)(unsigned char **args,int arg_num);
  void (*lcd_init)(unsigned char **args,int arg_num);

  void (*get_lcd_Xsize)(unsigned char **args,int arg_num);
  void (*get_lcd_Ysize)(unsigned char **args,int arg_num);
  void (*device_on)(unsigned char **args,int arg_num);
  void (*device_off)(unsigned char **args,int arg_num);
  void (*set_contrast)(unsigned char **args,int arg_num);
  void (*set_brightness)(unsigned char **args,int arg_num);
  void (*set_display_page)(unsigned char **args,int arg_num);
  void (*write_page)(unsigned char **args,int arg_num);

  void (*clear_screen)(unsigned char **args,int arg_num);
  void (*fill_screen)(unsigned char **args,int arg_num);
  void (*set_front_color)(unsigned char **args,int arg_num);
  void (*get_front_color)(unsigned char **args,int arg_num);
  void (*set_back_color)(unsigned char **args,int arg_num);
  void (*get_back_color)(unsigned char **args,int arg_num);
  void (*draw_point)(unsigned char **args,int arg_num);
  void (*draw_line)(unsigned char **args,int arg_num);
  void (*draw_rectangle)(unsigned char **args,int arg_num);
  void (*draw_cricle)(unsigned char **args,int arg_num);
  void (*d_round_rectangle)(unsigned char **args,int arg_num);
  void (*fill_rectangle)(unsigned char **args,int arg_num);
  void (*fill_cricle)(unsigned char **args,int arg_num);
  void (*f_round_rectangle)(unsigned char **args,int arg_num);
  void (*print_string)(unsigned char **args,int arg_num);
  void (*print_integer)(unsigned char **args,int arg_num);
  void (*print_float)(unsigned char **args,int arg_num);
  void (*set_font)(unsigned char **args,int arg_num);
  void (*get_font)(unsigned char **args,int arg_num);
  void (*get_font_Xsize)(unsigned char **args,int arg_num);
  void (*get_font_Ysize)(unsigned char **args,int arg_num);
  void (*draw_bitmap)(unsigned char **args,int arg_num);
};

/**
********************************************************************************
* @variable  lcd
* @brief     Global variable name for object_lcd
*            Applications can use either the 'lcd' here or some other names
*            when needed.
*            These 'lcd' are not initialized by default.
********************************************************************************
**/
#define GLOBAL_LCD_NAME

#ifdef GLOBAL_LCD_NAME
object_lcd global_lcd;
#endif


/**
********************************************************************************
*  @function    int object_lcd_init(object_lcd * lcd);
*  @brief       Set up lcd object interface.
*               Here , all the actions are link to their real functions.
*  @return      return 0  --> succeed
*               return -1 --> fail
********************************************************************************
**/
extern int object_lcd_init(object_lcd * lcd);

#endif
