/**
********************************************************************************
* @file     lcd_object.c
* @author   
* @version  v0.1
* @date     2014.11.27
* @brief    This file contains all functions that support object_lcd interface
*           Note: some functions need initialized global 'usart1'
********************************************************************************
**/

#include <string.h>      /* strncmp() */
#include <stm32f10x.h>   /* gpio functions */
#include "delay.h"       /* Delayms(__IO uint32_t nTime) */
#include "FSMCDriver.h"  /* FSMC_Init() */
#include "uart_object.h" /* object_uart interface & global 'usart1' */
#include "lcd_object.h"  /* object_lcd interface */
#include "DefaultFonts.h"


/**
********************************************************************************
* @array    lcd_index[]
* @brief    LCD type index.
*           LCD in this array can be used.
********************************************************************************
**/

struct _lcd_type 
{
  const char name[8];
  unsigned int x_size;
  unsigned int y_size;
};

#define MAX_LCD_INDEX_NUM 1

static const struct _lcd_type lcd_index[] =
  {
    { "HX8347A", 240 , 320}    /* HX8347A , Xsize = 240 ,Ysize = 320*/
  };


/**
********************************************************************************
* @variable front_color , back_color
*           display_x , display_y
*           cfont
* @brief    Global LCD information variables in this file. (Only in this file)
*           front_color  --> color for drawn area, 0~65535 
*           back_color   --> color for blank area, 0~65525
*           display_x    --> the max x_size of the current LCD
*           display_y    --> the max y_size of the current LCD
*           cfont        --> the current font to be used
********************************************************************************
**/

static unsigned int front_color = 0,back_color = 65535;
static unsigned int display_x = 0;
static unsigned int display_y = 0;

struct _current_font
{
  const unsigned char *font; /* font name in file 'DefaultFonts.h' */
  unsigned int x_size;       /* font Xsize */
  unsigned int y_size;       /* font Ysize */
  unsigned int offset;       /* used to calculate the location of a character */
  unsigned int numchars;     /* characters' number in current font array */
};

static struct _current_font cfont = {SmallFont , 0x08, 0x0c, 0x20, 0x5f};  


/**
********************************************************************************
* @micro    Bank1_LCD_D 
*           Bank1_LCD_C 
* @brief    Hardware related :
*           GPIO PD2 --> LCD_RST
*           GPIO PD3 --> LCD_ON
*           STM32F103F use "FSMC" to communicate with LCD controller.
*           In this way ,the LCD Data_addr is link to memory_addr 0x60020000
*           the LCD Reg_addr is link to memory_addr 0x60000000
********************************************************************************
**/
#define Bank1_LCD_D    ((uint32_t)0x60020000)    //disp Data ADDR
#define Bank1_LCD_C    ((uint32_t)0x60000000)    //disp Reg ADDR


/**
********************************************************************************
* @function  unsigned int RGB_to_color(int r ,int g ,int b)
* @brief     translate (RGB) data to (unsigned int) data.
*            RGB 16bit: R(5)G(6)B(5)
********************************************************************************
**/
static unsigned int RGB_to_color(unsigned int r ,unsigned int g ,unsigned int b)
{
  unsigned int color;  
  color =  (r & 248) <<8 | (g & 252) <<3 | (b & 248) >>3;
  return color;
}


/**
********************************************************************************
* @function  void Write_Command(unsigned int index)
* @brief     Send a value to LCD Reg_Address
********************************************************************************
**/
static void Write_Command(unsigned int index)
{
    *(__IO uint16_t *) (Bank1_LCD_C)= index;
}

/**
********************************************************************************
* @function  void Write_Command_Data(unsigned int index,unsigned int val)
* @brief     Send a value to LCD Reg_Address and follow a data to Data_Address
********************************************************************************
**/
static void Write_Command_Data(unsigned int index,unsigned int val)
{
  *(__IO uint16_t *) (Bank1_LCD_C)= index;
  *(__IO uint16_t *) (Bank1_LCD_D)= val;

}

/**
********************************************************************************
* @function  void  Write_Data(unsigned int val)
* @brief     Send a value to LCD Data_Address
********************************************************************************
**/
static void  Write_Data(unsigned int val)
{   
  *(__IO uint16_t *) (Bank1_LCD_D)= val;

}

/**
********************************************************************************
* @function  unsigned int str_to_uint(unsigned char *str)
* @brief     Change a string to integer. Like " int atoi(const char*) "
*            For example :
*             str=" 1 2 3" --> (int)num=str_to_uint(str) --> num = 123
*            Note: only string like "0123 456789" can be transformed.[123456789]
* @return    num --> transformed integer 
********************************************************************************
**/

static unsigned int str_to_uint(unsigned char *str)
{
  unsigned int num;

  unsigned char *p;
  p = str;
  num = 0;
  while(*p != '\0')
  {
	  if( *p > '/' && *p < ':') /* *p = '0' ~ '9' */
	  {
		  num = (num << 3) + (num << 1) + *p - '0'; /* num = num * 10 + next_bit*/
	  }

    p++;
  }
  return num;
}


/**
********************************************************************************
* @function  void SetXY(unsigned int x0,unsigned int y0,
*	                unsigned int x1,unsigned int y1)
* @brief     Set an area to be drawn.
********************************************************************************
**/
static void SetXY(unsigned int x0,unsigned int y0,
		  unsigned int x1,unsigned int y1)
{

  Write_Command_Data(0x0044,(x1 << 8) + x0);
  Write_Command_Data(0x0045,y0);
  Write_Command_Data(0x0046,y1);
  Write_Command_Data(0x004e,x0);
  Write_Command_Data(0x004f,y0);
  Write_Command (0x0022);//LCD_WriteCMD(GRAMWR);
}


/**
********************************************************************************
* @function  void LCD_Init(unsigned char **args, int arg_num)
* @brief     Initialize LCD (HX8347A) ,ignore 'args' 
********************************************************************************
**/
static void LCD_Init_hx8347a(unsigned char **args, int arg_num)
{
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
  unsigned char *p = *args;
  if(arg_num != 0) /* This function does not need parameters */
    {
      p++;
      return ;
    }

  GPIO_ResetBits(GPIOD, GPIO_Pin_2);
  Delayms(4);					   
  GPIO_SetBits(GPIOD, GPIO_Pin_2 );		 	 
  Delayms(10);

  Write_Command_Data(0x0000,0x0001);
  Write_Command_Data(0x0003,0xA8A4);
  Write_Command_Data(0x000C,0x0000);
  Write_Command_Data(0x000D,0x080C);
  Write_Command_Data(0x000E,0x2B00);
  Write_Command_Data(0x001E,0x00B7);
  Write_Command_Data(0x0001,0x2B3F);
  Write_Command_Data(0x0002,0x0600);
  Write_Command_Data(0x0010,0x0000);
  Write_Command_Data(0x0011,0x6070);
  Write_Command_Data(0x0005,0x0000);
  Write_Command_Data(0x0006,0x0000);
  Write_Command_Data(0x0016,0xEF1C);
  Write_Command_Data(0x0017,0x0003);
  Write_Command_Data(0x0007,0x0233);
  Write_Command_Data(0x000B,0x0000);
  Write_Command_Data(0x000F,0x0000);
  Write_Command_Data(0x0041,0x0000);
  Write_Command_Data(0x0042,0x0000);
  Write_Command_Data(0x0048,0x0000);
  Write_Command_Data(0x0049,0x013F);
  Write_Command_Data(0x004A,0x0000);
  Write_Command_Data(0x004B,0x0000);
  Write_Command_Data(0x0044,0xEF00);
  Write_Command_Data(0x0045,0x0000);
  Write_Command_Data(0x0046,0x013F);
  Write_Command_Data(0x0030,0x0707);
  Write_Command_Data(0x0031,0x0204);
  Write_Command_Data(0x0032,0x0204);
  Write_Command_Data(0x0033,0x0502);
  Write_Command_Data(0x0034,0x0507);
  Write_Command_Data(0x0035,0x0204);
  Write_Command_Data(0x0036,0x0204);
  Write_Command_Data(0x0037,0x0502);
  Write_Command_Data(0x003A,0x0302);
  Write_Command_Data(0x003B,0x0302);
  Write_Command_Data(0x0023,0x0000);
  Write_Command_Data(0x0024,0x0000);
  Write_Command_Data(0x0025,0x8000);
  Write_Command_Data(0x004f,0x0000);
  Write_Command_Data(0x004e,0x0000);
  Write_Command(0x0022);
 if(usart1.debug == 1)
   {
     usart1.printf(&usart1,"LCD is initialized.\n");
   }
}



/**
********************************************************************************
* @function  void FillScreen(unsigned char **args, int arg_num)
* @brief     Fill the LCD screen with the color passed by the parameter 'args'.
********************************************************************************
**/
static void FillScreen(unsigned char **args, int arg_num)
{
  unsigned int i,j;
  unsigned int tmp[3];  /* store the color or RGB data*/
  unsigned int color;

  switch(arg_num)  /* set color ,depends on arg_num*/
    {
    case 1:        /* 1 parameter --> color */
      tmp[0] = str_to_uint(*args); /* change string to integer */
      color  = tmp[0];
      break;
    case 3:        /* 3 parameters , RGB data --> color */
      for(i = 0 ; i < 3 ; i++)
	{
	  tmp[i] = str_to_uint(*(args + i));
	}
      color = RGB_to_color(tmp[0],tmp[1],tmp[2]);
      break;
    case 0:        /* no parameter , set default value */
    default:       /* wrong parameters , set default value */
      color = back_color;
      break;
    }

  SetXY(0, 0, display_x - 1, display_y - 1);

  for(i = 0; i < display_y; i++)
    {
      for (j = 0; j < display_x; j++)
	{
	  Write_Data(color);
	}

    }		
}

/**
********************************************************************************
* @function  void ClearScreen(unsigned char **args, int arg_num)
* @brief     Fill the LCD screen with the back_color.
********************************************************************************
**/
static void ClearScreen(unsigned char **args, int arg_num)
{
  unsigned int i,j;

  if(arg_num != 0)
    {
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
      unsigned char *p = *args;
      p++;
      return ;
    }
  SetXY(0, 0, display_x - 1, display_y - 1);

  for(i = 0; i < display_y; i++)
    {
      for (j = 0; j < display_x; j++)
	{
	  Write_Data(back_color);
	}
    }		
}


/**
********************************************************************************
* @function  void SetFrontColor(unsigned char **args, int arg_num)
* @brief     Front Color is used for area to be drawn.
********************************************************************************
**/
static void SetFrontColor(unsigned char **args, int arg_num)
{
  unsigned int color_r,color_g,color_b;

  switch(arg_num) /* set front_color ,depends on args */
    {
    case 1:  /* 1 parameters */
      front_color = str_to_uint(*args);
      break;
    case 3: /* 3 parameters , RGB */
      color_r = str_to_uint(*args);
      color_g = str_to_uint(*(args+1));
      color_b = str_to_uint(*(args+2));
      front_color = RGB_to_color(color_r,color_g,color_b);
      break;
    default:
      break;
    }
}


/**
********************************************************************************
* @function  void GetFrontColor(unsigned char **args, int arg_num)
* @brief     print the front_color to the usart1.
********************************************************************************
**/
static void GetFrontColor_uart1(unsigned char **args, int arg_num)
{
  if(arg_num != 0)
    {
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
      unsigned char *p = *args;
      p++;
      return;
    }
  usart1.printf(&usart1,"front_color is %d\n",front_color);
}


/**
********************************************************************************
* @function  void SetBackColor(unsigned char **args, int arg_num)
* @brief     Back color is for blank area.
********************************************************************************
**/
static void SetBackColor(unsigned char **args, int arg_num)
{
  unsigned int color_r,color_g,color_b;

  switch(arg_num) /* set back_color ,depends on args */
    {
    case 1:  /* 1 parameters */
      back_color = str_to_uint(*args);
      break;
    case 3: /* 3 parameters , RGB */
      color_r = str_to_uint(*args);
      color_g = str_to_uint(*(args+1));
      color_b = str_to_uint(*(args+2));
      back_color = RGB_to_color(color_r,color_g,color_b);
      break;
    default:
      break;      
    }
}

/**
********************************************************************************
* @function  void GetBackColor(unsigned char **args, int arg_num)
* @brief     print the back_color to the usart1.
********************************************************************************
**/
static void GetBackColor_uart1(unsigned char **args, int arg_num)
{
  if(arg_num != 0)
    {
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
      unsigned char *p = *args;
      p++;
      return;
    }
  usart1.printf(&usart1,"back_color is %d\n",back_color);
}

/**
********************************************************************************
* @function  void _DrawPoint(unsigned int x,unsigned int y)
* @brief     Draw a point.
********************************************************************************
**/
static void _DrawPoint(unsigned int x,unsigned int y)
{
  SetXY(x, y, x, y);
  Write_Data(front_color);
}

/**
********************************************************************************
* @function  void DrawPoint(unsigned char **args ,int arg_num)
* @brief     Get the point location and call the _DrawPoint.
********************************************************************************
**/
static void DrawPoint(unsigned char **args ,int arg_num)
{
  unsigned int x,y;
  if(arg_num == 2)
    {
      x = str_to_uint(*args);
      y = str_to_uint(*(args + 1));
      if(x < display_x && y < display_y)
	{
	  _DrawPoint(x , y);
	}
    }
}

/**
********************************************************************************
* @function  static void _DrawLine_H(unsigned int x0, unsigned int y0 ,
*			             unsigned int x1 ,unsigned int y1)
* @brief     Draw a horizontal line.
********************************************************************************
**/
static void _DrawLine_H(unsigned int x0, unsigned int y0 ,
			unsigned int x1 ,unsigned int y1)
{
  unsigned int i ,l;
  /* variable p is only used to avoid warning 'unused parameter'*/
  unsigned int p = y1;
  p++;

  if(x0 > x1)
    {
      SetXY(x1,y0,x0,y0);
      l = x0 -x1;
    }
  else
    {
      SetXY(x0,y0,x1,y0);
      l = x1 - x0;
    }

  for(i = 0; i < l; i++)
    {
      Write_Data(front_color);
    }

}

/**
********************************************************************************
* @function  _DrawLine_V(unsigned int x0, unsigned int y0 ,
			unsigned int x1 ,unsigned int y1)
* @brief     Draw a vertical line.
********************************************************************************
**/
static void _DrawLine_V(unsigned int x0, unsigned int y0 ,
			unsigned int x1 ,unsigned int y1)
{
  unsigned int i,l;
  /* variable p is only used to avoid warning 'unused parameter'*/
  unsigned int p = x1;
  p++;

  if(y0 > y1)
    {
      SetXY(x0,y1,x0,y0);      
      l = y0 - y1;
    }
  else
    {
      SetXY(x0,y0,x0,y1);      
      l = y1 - y0;
    }

  for(i = 0; i < l; i++)
    {
      Write_Data(front_color);
    }
}

/**
********************************************************************************
* @function  void _DrawLine_A(unsigned int x0, unsigned int y0 ,
*                	       unsigned int x1 ,unsigned int y1)
* @brief     Draw an ablique line.
********************************************************************************
**/
static void _DrawLine_A(unsigned int x0, unsigned int y0 ,
			unsigned int x1 ,unsigned int y1)
{
  int i, delta_x, delta_y, numpixels, d, dinc1, dinc2,
    x, xinc1, xinc2, y, yinc1, yinc2;
    
  /* Calculate delta-x and delta-y for initialization */
  delta_x = (x1 > x0)?(x1 - x0):(x0 - x1);
  delta_y = (y1 > y0)?(y1 - y0):(y0 - y1);

  /* Initialize all vars based on which is the independent variable */ 
  if(delta_x >= delta_y)
    {
      /* x is independent variable */
      numpixels = delta_x + 1;
      d = (delta_y << 1) - delta_x;
      dinc1 = delta_y << 1;
      dinc2 = (delta_y - delta_x) << 1;
      xinc1 = 1;
      xinc2 = 1;
      yinc1 = 0;
      yinc2 = 1;
    }
  else
    {
      /* y is independent variable */ 
      numpixels = delta_y + 1;
      d = (delta_x << 1) - delta_y;
      dinc1 = delta_x << 1;
      dinc2 = (delta_x - delta_y) << 1;
      xinc1 = 0;
      xinc2 = 1;
      yinc1 = 1;
      yinc2 = 1;
    }
  // Make sure x and y move in the right directions
  //
  if(x0 > x1)
    {
      xinc1 = - xinc1;
      xinc2 = - xinc2;
    }
  if(y0 > y1)
    {
      yinc1 = - yinc1;
      yinc2 = - yinc2;
    }

  // Start drawing at <x1, y1>
  //
  x = x0;
  y = y0;
  // Draw the points
  //
  for(i= 1; i< numpixels;i++)
    {
      SetXY(x,y,x,y);
      Write_Data(front_color);
      if (d < 0)
	{
	  d = d + dinc1;
	  x = x + xinc1;
	  y = y + yinc1;
	}
      else
	{
	  d = d + dinc2;
	  x = x + xinc2;
	  y = y + yinc2;
	}
    }
}

/**
********************************************************************************
* @function  void void _TestDrawLine()
* @brief     print some lines on the screen.
********************************************************************************
**/
static void _TestDrawLine(void)
{
  unsigned int location = 20;

  _DrawLine_H(20,150,220,150);
  _DrawLine_V(120,10,120,310);
  while(location < display_y)
    {
      _DrawLine_A(20,150,120,location);
      location += 25;
    }
}


/**
********************************************************************************
* @function  void DrawLine(unsigned char **args ,int arg_num)
* @brief     Get the line parameters and call the draw_line functions.
********************************************************************************
**/
static void DrawLine(unsigned char **args ,int arg_num)
{
  unsigned int location[4] ,count;
  
  if(arg_num != 4)
    {
#ifdef LCD_DEBUG
      if(arg_num == 0)
	{
	  _TestDrawLine();
	  return;
	}
#endif
      return;      
    }

  for(count = 0 ;count < 4; count++)
    {
      location[count] = str_to_uint(*(args + count));
    }

  /* location out of range ,return*/
  if(location[0] >= display_x || location[2] >= display_x ||
     location[1] >= display_y || location[3] >= display_y)
    {
      return ;
    }

  if(location[0] == location[2])    /* Draw a vertical line.*/
    {
      _DrawLine_V(location[0],location[1],location[2],location[3]);
    }
  else if(location[1] == location[3])    /* Draw a horizontal line.*/
    {
      _DrawLine_H(location[0],location[1],location[2],location[3]);
    }
  else /* Draw an ablique line.*/
    {
      _DrawLine_A(location[0],location[1],location[2],location[3]);
    }
}

/**
********************************************************************************
* @function  _DrawRectangle(unsigned int x0, unsigned int y0,
		            unsigned int x1, unsigned int y1)
* @brief     Draw a Rectangle.
********************************************************************************
**/
static void _DrawRectangle(unsigned int x0, unsigned int y0,
			   unsigned int x1, unsigned int y1)
{
  _DrawLine_H(x0 ,y0 ,x1 ,y0);
  _DrawLine_H(x0 ,y1 ,x1 ,y1);
  _DrawLine_V(x0 ,y0 ,x0 ,y1);
  _DrawLine_V(x1 ,y0 ,x1 ,y1);
  _DrawPoint(x1,y1);
}

/**
********************************************************************************
* @function  void _Test_DrawRectangle(void)
* @brief     print some Rectangles on LCD
********************************************************************************
**/
static void _Test_DrawRectangle(void)
{
  unsigned int x0 = 0,y0 = 0 ,x1 = display_x - 1 ,y1 = display_y - 1;
  
  while( (x0 < x1) && (y0 < y1) )
    {
      _DrawRectangle(x0 ,y0 ,x1 ,y1);
      x0 += 10;
      x1 -= 10;
      y0 += 10;
      y1 -= 10;
    }
}

/**
********************************************************************************
* @function  void DrawRectangle(unsigned char **args ,int arg_num)
* @brief     Get the rectangle parameters and call the _DrawRectangle function
********************************************************************************
**/
static void DrawRectangle(unsigned char **args ,int arg_num)
{
  unsigned int location[4] ,count;
  
  if(arg_num != 4)
    {
#ifdef LCD_DEBUG
      if(arg_num == 0)
	{
	  _Test_DrawRectangle();
	  return;
	}
#endif
      return;      
    }

  for(count = 0 ;count < 4; count++)
    {
      location[count] = str_to_uint(*(args + count));
    }

  /* location out of range ,return*/
  if(location[0] >= display_x || location[2] >= display_x || 
     location[1] >= display_y || location[3] >= display_y)
    {
      return ;
    }

  _DrawRectangle(location[0],location[1],location[2],location[3]);
}


/**
********************************************************************************
* @function  void _Draw_Circle(int x, int y ,int radius)
* @brief     Draw a circle.
********************************************************************************
**/
static void _Draw_Circle(int x, int y ,int radius)
{
  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x1 = 0;
  int y1 = radius;

  _DrawPoint( x, y + radius);    
  _DrawPoint( x, y - radius);    
  _DrawPoint( x + radius, y);    
  _DrawPoint( x - radius, y);    

  while(x1 < y1)
    {
      if(f >= 0) 
	{
	  y1--;
	  ddF_y += 2;
	  f += ddF_y;
	}
      x1++;
      ddF_x += 2;
      f += ddF_x;    

      _DrawPoint( x + x1, y + y1);    
      _DrawPoint( x - x1, y + y1);    
      _DrawPoint( x + x1, y - y1);    
      _DrawPoint( x - x1, y - y1);    

      _DrawPoint( x + y1, y + x1);
      _DrawPoint( x - y1, y + x1);    
      _DrawPoint( x + y1, y - x1);    
      _DrawPoint( x - y1, y - x1);    
    }
}

/**
********************************************************************************
* @function  _Test_DrawCircle(void)
* @brief     print some circles on LCD
********************************************************************************
**/
static void _Test_DrawCircle(void)
{
  unsigned int x = 120, y = 160 , r = 10;
  int tmp[4];


  tmp[0] = (int) x - (int) r;
  tmp[1] = (int) x + (int) r;
  tmp[2] = (int) y - (int) r;
  tmp[3] = (int) y + (int) r;

  while(tmp[0] >= 0 && tmp[1] < (int)display_x && 
	tmp[2] >= 0 && tmp[3] < (int)display_y)
    {
      _Draw_Circle( x, y, r);
      r += 10;
      tmp[0] = (int) x - (int) r;
      tmp[1] = (int) x + (int) r;
      tmp[2] = (int) y - (int) r;
      tmp[3] = (int) y + (int) r;
    }
}

/**
********************************************************************************
* @function  void DrawCircle(unsigned char **args ,int arg_num)
* @brief     Get parameters and call the _DrawCircle function
********************************************************************************
**/
static void DrawCircle(unsigned char **args ,int arg_num)
{
  int tmp[3]; /* (tmp[0],tmp[1]; r = tmp[2]) */
  unsigned int count;
  if(arg_num != 3)
    {
#ifdef LCD_DEBUG
      if(arg_num == 0)
	{
	  _Test_DrawCircle();
	  return;
	}
#endif
      return;      
    }

  for(count = 0 ;count < 3; count++)
    {
      tmp[count] = (int)str_to_uint(*(args + count));
    }

  /* location out of range ,return*/
  if(tmp[0] >= (int)display_x || tmp[1] >= (int)display_y ||
     tmp[0] + tmp[2] >= (int)display_x || 
     tmp[0] - tmp[2] <  0 || 
     tmp[1] + tmp[2] >= (int)display_y || 
     tmp[1] - tmp[2] <  0   )   
    {
      return ;
    }
  
  _Draw_Circle(tmp[0], tmp[1], tmp[2]);
}

/**
********************************************************************************
* @function  _DrawRRect(unsigned int x0 ,unsigned int y0 ,
		       unsigned int x1 ,unsigned int y1 , unsigned int radius)
* @brief     Draw a round corner rectangle
********************************************************************************
**/
static void _DrawRRect(unsigned int x0 ,unsigned int y0 ,
		       unsigned int x1 ,unsigned int y1 , unsigned int radius)
{
  int f = 1 - (int)radius;
  int ddF_x = 1;
  int ddF_y = -2 * (int)radius;
  int step_x = 0;
  int step_y = (int)radius;
  
  unsigned int x = x0 + radius;
  unsigned int y = y0 + radius;
  unsigned int offset_x = x1 - x0 - (radius << 1);
  unsigned int offset_y = y1 - y0 - (radius << 1);

  while(step_x < step_y)
    {
      if(f >= 0) 
	{
	  step_y--;
	  ddF_y += 2;
	  f += ddF_y;
	}
      step_x++;
      ddF_x += 2;
      f += ddF_x;    

      _DrawPoint( x - step_x, y - step_y);    
      _DrawPoint( x - step_x, y + step_y + offset_y);    
      _DrawPoint( x + step_x + offset_x, y + step_y + offset_y);    
      _DrawPoint( x + step_x + offset_x, y - step_y);    

      _DrawPoint( x - step_y, y - step_x);    
      _DrawPoint( x - step_y, y + step_x + offset_y);    
      _DrawPoint( x + step_y + offset_x, y + step_x + offset_y);
      _DrawPoint( x + step_y + offset_x, y - step_x);    
    }
  _DrawLine_H((x0 + radius), y0, (x1 - radius), y0);
  _DrawLine_H((x0 + radius), y1, (x1 - radius), y1);
  _DrawLine_V(x0, (y0 + radius), x0, (y1 - radius));
  _DrawLine_V(x1, (y0 + radius), x1, (y1 - radius));

}


/**
********************************************************************************
* @function  void _Test_DrawRoundRect(void)
* @brief     Draw some round corner rectangles.
********************************************************************************
**/
static void _Test_DrawRoundRect(void)
{
  unsigned int x0 = 0,y0 = 0 ,x1 = display_x - 1 ,y1 = display_y - 1;
  unsigned int radius = 60;
  while( (x0 < x1) && (y0 < y1) && (radius >= 5))
    {
      _DrawRRect(x0 ,y0 ,x1 ,y1, radius);
      x0 += 10;
      x1 -= 10;
      y0 += 10;
      y1 -= 10;
      radius = (x1 - x0) >> 2;
    }
}

/**
********************************************************************************
* @function  void DrawRectangle(unsigned char **args ,int arg_num)
* @brief     Get parameters and call _DrawRRect functions.
********************************************************************************
**/
static void DrawRoundRect(unsigned char **args ,int arg_num)
{
  unsigned int location[5] ,count ,tmp;
  unsigned int delta_x,delta_y;
  
  if(arg_num != 5 && arg_num != 4)
    {
#ifdef LCD_DEBUG
      if(arg_num == 0)
	{
	  _Test_DrawRoundRect();
	  return;
	}
#endif
      return;      
    }

  for(count = 0 ;count < (unsigned int)arg_num; count++)
    {
      location[count] = str_to_uint(*(args + count));
    }

  /* location out of range. */
  if(location[0] > display_x || location[2] > display_x || 
     location[1] > display_y || location[3] > display_y)
    {
      return ;
    }

  if(location[0] > location[2])
    {
      tmp = location[0];
      location[0] = location[2];
      location[2] = tmp;
    }

  if(location[1] > location[3])
    {
      tmp = location[1];
      location[1] = location[3];
      location[3] = tmp;
    }
  
  delta_x = location[2] - location[0];
  delta_y = location[3] - location[1];

  if(arg_num == 4)
    {
      location[4] = (delta_x < delta_y)?(delta_x >> 2):(delta_y >> 2);
    }
  else if((location[4] << 1) > delta_x || (location[4] << 1) > delta_y)
    {
      return;
    }

  _DrawRRect(location[0],location[1],location[2],location[3],location[4]);
}


/**
********************************************************************************
* @function  void _FillRectangle(unsigned int x0 , unsigned int y0 ,
*                                unsigned int x1 , unsigned int y1)
* @brief     Draw a filled Rectangle
********************************************************************************
**/
static void _FillRectangle(unsigned int x0 , unsigned int y0 ,
			   unsigned int x1 , unsigned int y1)
{
  while(y0 <= y1)
    {
      _DrawLine_H(x0 ,y0 ,x1 ,y0);
      y0++;
    }
}

/**
********************************************************************************
* @function  void FillRectangle(unsigned char **args ,int arg_num)
* @brief     Get parameters and call the _FillRectagnle function.
********************************************************************************
**/
static void FillRectangle(unsigned char **args ,int arg_num)
{
  unsigned int location[4] ,count ,tmp;
  
  if(arg_num != 4)
    {
      return;      
    }

  for(count = 0 ;count < 4; count++)
    {
      location[count] = str_to_uint(*(args + count));
    }

  /*location out of range ,return*/
  if(location[0] > display_x || location[2] > display_x || 
     location[1] > display_y || location[3] > display_y)
    {
      return ;
    }

  if(location[1] > location[3])
    {
      tmp = location[1];
      location[1] = location[3];
      location[3] = tmp;
    }

  _FillRectangle(location[0],location[1],location[2],location[3]);
}

/**
********************************************************************************
* @function  void _Fill_Circle(int x, int y ,int radius)
* @brief     Draw a filled circle
********************************************************************************
**/
static void _Fill_Circle(int x, int y ,int radius)
{
  int f = 1 - radius;
  int ddF_x = 1;
  int ddF_y = -2 * radius;
  int x1 = 0;
  int y1 = radius;

  _DrawLine_H( (x - radius), y, (x + radius), y);
  _DrawLine_V( x, (y - radius), x, (y + radius));

  while(x1 < y1)
    {
      if(f >= 0) 
	{
	  y1--;
	  ddF_y += 2;
	  f += ddF_y;
	}
      x1++;
      ddF_x += 2;
      f += ddF_x;    

      _DrawLine_H( (x - x1), y + y1, (x + x1), y + y1);
      _DrawLine_H( (x - x1), y - y1, (x + x1), y - y1);

      _DrawLine_H( (x - y1), y + x1, (x + y1), y + x1);
      _DrawLine_H( (x - y1), y - x1, (x + y1), y - x1);

    }
}


/**
********************************************************************************
* @function  void FillCircle(unsigned char **args ,int arg_num)
* @brief     Get parameters and call the _Fill_Circle function
********************************************************************************
**/
static void FillCircle(unsigned char **args ,int arg_num)
{
  int tmp[3] ,count; /* (tmp[0],tmp[1]; r = tmp[2]) */

  if(arg_num != 3)
    {
      return;      
    }

  for(count = 0 ;count < 3; count++)
    {
      tmp[count] = (int)str_to_uint(*(args + count));
    }

  /*location out of range*/
  if(tmp[0] >= (int)display_x || tmp[1] >= (int)display_y ||
     tmp[0] + tmp[2] >= (int)display_x || 
     tmp[0] - tmp[2] <  0   || 
     tmp[1] + tmp[2] >= (int)display_y || 
     tmp[1] - tmp[2] <  0   )   
    {
      return ;
    }
  _Fill_Circle(tmp[0],tmp[1],tmp[2]);
}

/**
********************************************************************************
* @function  _FillRoundRect(unsigned int x0 ,unsigned int y0 ,
			   unsigned int x1 ,unsigned int y1 , unsigned int radius)
* @brief     Draw a filled round corner rectangle.
********************************************************************************
**/
static void _FillRoundRect(unsigned int x0 ,unsigned int y0 ,
			   unsigned int x1 ,unsigned int y1 , unsigned int radius)
{
  int f = 1 - (int)radius;
  int ddF_x = 1;
  int ddF_y = -2 * (int)radius;
  int step_x = 0;
  int step_y = (int)radius;
  
  unsigned int x = x0 + radius;
  unsigned int y = y0 + radius;
  unsigned int offset_x = x1 - x0 - (radius << 1);
  unsigned int offset_y = y1 - y0 - (radius << 1);

  while(step_x < step_y)
    {
      if(f >= 0) 
	{
	  step_y--;
	  ddF_y += 2;
	  f += ddF_y;
	}
      step_x++;
      ddF_x += 2;
      f += ddF_x;    

      _DrawLine_H( x - step_x, y - step_y, x + step_x + offset_x, y - step_y);
      _DrawLine_H( x - step_y, y - step_x, x + step_y + offset_x, y - step_x);
      _DrawLine_H( x - step_x, y + step_y + offset_y,
		   x + step_x + offset_x, y + step_y + offset_y);
      _DrawLine_H( x - step_y, y + step_x + offset_y,
		   x + step_y + offset_x, y + step_x + offset_y);
    }

  while(y0 + radius <= y1 - radius )
    {
      _DrawLine_H( x0, y0 + radius, x1, y0 + radius);
      y0++;
    }
}

/**
********************************************************************************
* @function  void FillRoundRect(unsigned char **args ,int arg_num)
* @brief     Get parameters and call the _FillRoundRect function.
********************************************************************************
**/
static void FillRoundRect(unsigned char **args ,int arg_num)
{
  unsigned int location[5] ,count ,tmp;
  unsigned int delta_x,delta_y;
  
  if(arg_num != 5 && arg_num != 4)
    {
      return;      
    }

  for(count = 0 ;count < (unsigned int)arg_num; count++)
    {
      location[count] = str_to_uint(*(args + count));
    }

  /*location out of range*/
  if(location[0] > display_x || location[2] > display_x || 
     location[1] > display_y || location[3] > display_y)
    {
      return ;
    }

  if(location[0] > location[2])
    {
      tmp = location[0];
      location[0] = location[2];
      location[2] = tmp;
    }

  if(location[1] > location[3])
    {
      tmp = location[1];
      location[1] = location[3];
      location[3] = tmp;
    }
  
  delta_x = location[2] - location[0];
  delta_y = location[3] - location[1];

  if(arg_num == 4)
    {
      location[4] = (delta_x < delta_y)?(delta_x >> 2):(delta_y >> 2);
    }
  else if((location[4] << 1) > delta_x || (location[4] << 1) > delta_y)
    {
      return;
    }

  _FillRoundRect(location[0],location[1],location[2],location[3],location[4]);
}

/**
********************************************************************************
* @function  _print_char(unsigned int x ,unsigned int y ,unsigned char c)
* @brief     print a character on LCD.
********************************************************************************
**/
static void _print_char(unsigned int x ,unsigned int y ,unsigned char c)
{
  unsigned char i,ch;
  unsigned int j;
  unsigned int array_count;

  SetXY(x, y, x + cfont.x_size - 1, y + cfont.y_size - 1);
  array_count=((c - cfont.offset )*((cfont.x_size/8)*cfont.y_size))+4;
  for(j=0;j<((cfont.x_size/8)*cfont.y_size);j++)
    {
      ch=*(cfont.font + array_count);
      for(i=0;i<8;i++)
	{   
	  if((ch&(1<<(7-i)))!=0)   
	    {
	      Write_Data(front_color);
	    } 
	  else
	    {
	      Write_Data(back_color);
	    }   
	}
      array_count++;
    }
}


/**
********************************************************************************
* @function  void PrintString(unsigned char **args, int arg_num)
* @brief     print a string on LCD.
********************************************************************************
**/
static void PrintString(unsigned char **args, int arg_num)
{
  unsigned int location[2] ,count;
  unsigned char *str;
  if(arg_num != 3)
    {
      return;      
    }

  for(count = 0 ;count < 2; count++)
    {
      location[count] = str_to_uint(*(args + count));
    }

  if(location[0] >= display_x - cfont.x_size || 
     location[1] >= display_y - cfont.y_size)
    {
      return;
    }

  str = *(args + 2);

  while(*str != '\0')
    {
      _print_char(location[0] ,location[1] , *str);
      location[0] += cfont.x_size;

      if(location[0] >= display_x - cfont.x_size)
	{
	  location[0] = 0;
	  location[1] += cfont.y_size;
	}
      if(location[1] >= display_y - cfont.y_size) 
	{
	  return ;
	}

      str++;
    }
}


/**
********************************************************************************
* @function  void SetFont(unsigned char **args, int arg_num)
* @brief     Choose a font. font in file "DefaultFonts.h"
********************************************************************************
**/
static void SetFont(unsigned char **args, int arg_num)
{
  unsigned char font_index;

  if(arg_num != 1)
    {
      return;
    }

  font_index = **args;

  switch(font_index)
    {
    case '3':
      cfont.font = SevenSegNumFont;
      break;
    case '2':
      cfont.font = BigFont;
      break;
    case '1':
    default:
      cfont.font = SmallFont;
      break;
    }

  cfont.x_size = *cfont.font;
  cfont.y_size = *(cfont.font + 1);
  cfont.offset = *(cfont.font + 2);
  cfont.numchars = *(cfont.font + 3);
}

/**
********************************************************************************
* @function  void GetFont_uart1(unsigned char **args, int arg_num)
* @brief     print the current font name to usart1
********************************************************************************
**/
static void GetFont_uart1(unsigned char **args, int arg_num)
{
  if(arg_num != 0)
    {
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
      unsigned char *p = *args;
      p++;
      return ;
    }

  if(cfont.font == SmallFont)
    {
      usart1.printf(&usart1,"The current font name is 'SmallFont'\n");
    }
  else if(cfont.font == BigFont)
    {
      usart1.printf(&usart1,"The current font name is 'BigFont'\n");
    }
  else if(cfont.font == SevenSegNumFont)
    {
      usart1.printf(&usart1,"The current font name is 'SevenSegNumFont'\n");
    }
  else
    {
      usart1.printf(&usart1,"The current font is unknown.\n");
    }

  usart1.printf(&usart1,"Font X_size is %d\n",cfont.x_size);
  usart1.printf(&usart1,"Font Y_size is %d\n",cfont.y_size);
  usart1.printf(&usart1,"Font characters num is %d\n",cfont.numchars);
}

/**
********************************************************************************
* @function  void GetFontXSize_uart1(unsigned char **args ,int arg_num)
* @brief     print current font xsize to usart1.
********************************************************************************
**/
static void GetFontXSize_uart1(unsigned char **args ,int arg_num)
{
  if(arg_num != 0)
    {
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
      unsigned char *p = *args;
      p++;
      return ;
    }
  usart1.printf(&usart1,"Current font X_size is %d\n",cfont.x_size);
}

/**
********************************************************************************
* @function  void GetFontYSize_uart1(unsigned char **args ,int arg_num)
* @brief     print current font xsize to usart1.
********************************************************************************
**/
static void GetFontYSize_uart1(unsigned char **args ,int arg_num)
{
  if(arg_num != 0)
    {
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
      unsigned char *p = *args;
      p++;
      return ;
    }
  usart1.printf(&usart1,"Current font Y_size is %d\n",cfont.y_size);
}


/**
********************************************************************************
* @function  void GetDisplayXSize_uart1(unsigned char **args ,int arg_num)
* @brief     print current LCD diplay area xsize to usart1.
********************************************************************************
**/
static void GetDisplayXSize_uart1(unsigned char **args ,int arg_num)
{
  if(arg_num != 0)
    {
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
      unsigned char *p = *args;
      p++;
      return ;
    }
  usart1.printf(&usart1,"Current LCD x_size is %d\n",display_x);
}

/**
********************************************************************************
* @function  void GetDisplayYSize_uart1(unsigned char **args ,int arg_num)
* @brief     print current LCD diplay area ysize to usart1.
********************************************************************************
**/
static void GetDisplayYSize_uart1(unsigned char **args ,int arg_num)
{
  if(arg_num != 0)
    {
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
      unsigned char *p = *args;
      p++;
      return ;
    }
  usart1.printf(&usart1,"Current LCD y_size is %d\n",display_y);
}

/**
********************************************************************************
* @function  void DeviceOn(unsigned char **args ,int arg_num)
* @brief     turn on the back light .
********************************************************************************
**/
static void DeviceOn(unsigned char **args ,int arg_num)
{
  if(arg_num != 0)
    {
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
      unsigned char *p = *args;
      p++;
      return ;
    }
 if(usart1.debug == 1)
   {
     usart1.printf(&usart1,"Device on\n");
   }
  GPIO_SetBits(GPIOD, GPIO_Pin_3);
}


/**
********************************************************************************
* @function  void DeviceOff(unsigned char **args ,int arg_num)
* @brief     turn off the back light.
********************************************************************************
**/
static void DeviceOff(unsigned char **args ,int arg_num)
{
  if(arg_num != 0)
    {
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
      unsigned char *p = *args;
      p++ ;
      return ;
    }
 if(usart1.debug == 1)
   {
     usart1.printf(&usart1,"Device off\n");
   }
  GPIO_ResetBits(GPIOD, GPIO_Pin_3);
}

/**
********************************************************************************
* @function  void idle_hx8347a(unsigned char **args ,int arg_num)
* @brief     Do nothing.
*            Some object_lcd interface functions which HX8347A does not support,
*            link to this function. (avoid idle function pointer)
********************************************************************************
**/
static void idle_hx8347a(unsigned char **args ,int arg_num)
{
  /* variable (pointer) p is only used to avoid warning 'unused parameter'*/
  unsigned char *p = *args;
  int tmp = arg_num;
  p++ ;
  tmp++;
 if(usart1.debug == 1)
   {
     usart1.printf(&usart1,"HX8347A does not support this function\n");
   }
}

#if 0
void DrewBitmap(void)
{
  ;
}
#endif


/**
********************************************************************************
* @function  int get_lcd_index(object_ATcmd *at_cmd)
* @brief     Get the lcd index in the array lcd_index
* @return    count  --> the lcd index in lcd_index
*            -1 --> fail
********************************************************************************
**/
static int get_lcd_index(char *lcd_name)
{
  int count;
  for(count = 0; count < MAX_LCD_INDEX_NUM ;count++)
    {
      /*check the first 7 chararcters*/
      if( strncmp((const char*)lcd_name ,
		  (const char*)&lcd_index[count] , 7) == 0)
	{
    	  return count;
	}
    }
  return -1;
}

  /**
********************************************************************************
* @function  void set_device(unsigned char **args ,int arg_num)
* @brief     Reset the LCD index in lcd_index[] 
*            This need a global object_lcd variable 'global_lcd' .
*            And it will reinitialize the LCD.
********************************************************************************
**/

static void SetDevice(unsigned char **args ,int arg_num)
{
  unsigned char * lcd_name;
  int index;
  if(arg_num != 1)
    {
      return ;
    }

  lcd_name = *args;

  index = get_lcd_index((char *)lcd_name);

  if(index == -1)
    {
      if(usart1.debug == 1)
	{
	  usart1.printf(&usart1,"The '%s' is not support\n",lcd_name);
	}
      return ;
    }
  if(usart1.debug == 1)
    {
      usart1.printf(&usart1,"Set lcd_index to '%s'\n",lcd_name);
    }
  global_lcd.index = index;
  object_lcd_init(&global_lcd);
}

/**
********************************************************************************
*  @function    int action_test(unsigned char **args, int arg_num)
*  @brief       print the parameters in one command.
*               Just for test.
********************************************************************************
**/
static void action_test(unsigned char **args, int arg_num)
{
  int count;
  if(usart1.debug == 1)
    {
      usart1.printf(&usart1,"This is object_lcd test action.\n");
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
*  @function    int object_lcd_init(object_lcd * lcd);
*  @brief       Set up lcd object interface.
*               Here , all the actions are link to their real functions.
*  @return      return 0  --> succeed
*               return -1 --> fail
********************************************************************************
**/
int object_lcd_init(object_lcd * lcd)
{
  FSMC_Init();

  switch(lcd->index)
    {
    case 0: /* HX8347A */
    default:
      display_x = lcd_index[0].x_size;
      display_y = lcd_index[0].y_size;

      lcd->lcd_init = LCD_Init_hx8347a;

      lcd->set_contrast = idle_hx8347a;
      lcd->set_brightness = idle_hx8347a;
      lcd->set_display_page = idle_hx8347a;
      lcd->write_page = idle_hx8347a;
      break;
    }

  lcd->set_device = SetDevice;

  lcd->get_lcd_Xsize = GetDisplayXSize_uart1;
  lcd->get_lcd_Ysize = GetDisplayYSize_uart1;
  lcd->device_on = DeviceOn;
  lcd->device_off = DeviceOff;

  lcd->clear_screen = ClearScreen;
  lcd->fill_screen = FillScreen;
  lcd->set_front_color = SetFrontColor;
  lcd->get_front_color = GetFrontColor_uart1;
  lcd->set_back_color = SetBackColor;
  lcd->get_back_color = GetBackColor_uart1;
  lcd->draw_point = DrawPoint;
  lcd->draw_line = DrawLine;
  lcd->draw_rectangle = DrawRectangle;
  lcd->draw_cricle = DrawCircle;
  lcd->d_round_rectangle = DrawRoundRect;
  lcd->fill_rectangle = FillRectangle;
  lcd->fill_cricle = FillCircle;
  lcd->f_round_rectangle = FillRoundRect;
  lcd->print_string = PrintString;
  lcd->print_integer = PrintString;
  lcd->print_float = PrintString;
  lcd->set_font = SetFont;
  lcd->get_font = GetFont_uart1;
  lcd->get_font_Xsize = GetFontXSize_uart1; 
  lcd->get_font_Ysize = GetFontYSize_uart1;
  lcd->draw_bitmap = action_test;

  lcd->lcd_init((unsigned char **)0,0);
  lcd->clear_screen((unsigned char **)0,0);
  return 0;
}
