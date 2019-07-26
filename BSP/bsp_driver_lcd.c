#include "bsp_driver_lcd.h"
#include "dma2d.h"
#include "fonts.h"
/*���ڴ洢��ǰѡ��������ʽ*/
static sFONT *LCD_Currentfonts;
/* ���ڴ洢��ǰ������ɫ�����屳����ɫ�ı���*/
static uint32_t CurrentTextColor   = 0x000000;
static uint32_t CurrentBackColor   = 0xFFFFFF;
/* ���ڴ洢���Ӧ���Դ�ռ� �� ��ǰѡ��Ĳ�*/
static uint32_t CurrentFrameBuffer = LCD_FRAME_BUFFER;
static uint32_t CurrentLayer = LCD_BACKGROUND_LAYER;
#define CR_MASK                     ((uint32_t)0xFFFCE0FC)  /* DMA2D CR Mask */
#define PFCCR_MASK                  ((uint32_t)0x00FC00C0)  /* DMA2D FGPFCCR Mask */
#define DEAD_MASK                   ((uint32_t)0xFFFF00FE)  /* DMA2D DEAD Mask */
/*����Һ�������ֲ�Ĳ�������*/
#define HBP  46		//HSYNC�����Ч����
#define VBP  23		//VSYNC�����Ч����

#define HSW   1		//HSYNC���
#define VSW   1		//VSYNC���

#define HFP  20		//HSYNCǰ����Ч����
#define VFP   22		//VSYNCǰ����Ч����


#define ZOOMMAXBUFF 16384
uint8_t zoomBuff[ZOOMMAXBUFF] = {0};	//�������ŵĻ��棬���֧�ֵ�128*128

/**
  * @brief  ѡ��Ҫ���ƵĲ�.
  * @param  Layerx: ѡ��Ҫ����ǰ����(��2��)���Ǳ�����(��1��)
  * @retval None
  */
void LCD_SetLayer(uint32_t Layerx)
{
  if (Layerx == LCD_BACKGROUND_LAYER)
  {
    CurrentFrameBuffer = LCD_FRAME_BUFFER; 
    CurrentLayer = LCD_BACKGROUND_LAYER;
  }
  else
  {
    CurrentFrameBuffer = LCD_FRAME_BUFFER + BUFFER_OFFSET;
    CurrentLayer = LCD_FOREGROUND_LAYER;
  }
}  
/**
  * @brief  Configure the transparency.
  * @param  transparency: specifies the transparency, 
  *         This parameter must range from 0x00 to 0xFF.
  * @retval None
  */
void LCD_SetTransparency(uint8_t transparency)
{
  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  {
    HAL_LTDC_SetAlpha(&hltdc,(uint32_t)LTDC_Layer1, transparency);
  }
  else
  {     
    HAL_LTDC_SetAlpha(&hltdc,(uint32_t)LTDC_Layer2, transparency);
  }
  HAL_LTDC_Reload(&hltdc,LTDC_RELOAD_IMMEDIATE);
}

FlagStatus DMA2D_GetFlagStatus(uint32_t DMA2D_FLAG)
{
  FlagStatus bitstatus = RESET;
  
  /* Check the parameters */
  assert_param(IS_DMA2D_GET_FLAG(DMA2D_FLAG));
  
  /* Check the status of the specified DMA2D flag */
  if (((DMA2D->ISR) & DMA2D_FLAG) != (uint32_t)RESET)
  {
    /* DMA2D_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* DMA2D_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the DMA2D_FLAG status */
  return bitstatus;
}
/* USER CODE BEGIN 1 */
/**
  * @brief  Clears the hole LCD.
  * @param  Color: the color of the background.
  * @retval None
  */
void LCD_Clear(uint32_t Color)
{
	uint32_t timeout=0; 
	/*hdma2d.Instance = DMA2D;
	hdma2d.Init.Mode = DMA2D_R2M;
	hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
	hdma2d.Init.OutputOffset = 0x0;
	if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_DMA2D_ConfigLayer(&hdma2d,CurrentLayer); //������
	HAL_DMA2D_Start(&hdma2d,Color,CurrentFrameBuffer,800,480);//��������
	HAL_DMA2D_PollForTransfer(&hdma2d,1000);//��������*/
	__HAL_RCC_DMA2D_CLK_ENABLE();	//ʹ��DM2Dʱ��
	DMA2D->CR&=~(DMA2D_CR_START);	//��ֹͣDMA2D
	DMA2D->CR=DMA2D_R2M;			//�Ĵ������洢��ģʽ
	DMA2D->OPFCCR=DMA2D_OUTPUT_RGB888;	//������ɫ��ʽ
	DMA2D->OOR=0x0;				//������ƫ�� 

	DMA2D->OMAR=CurrentFrameBuffer;				//����洢����ַ
	DMA2D->NLR=(480)|((800)<<16);	//�趨�����Ĵ���
	DMA2D->OCOLR=Color;						//�趨�����ɫ�Ĵ��� 
	DMA2D->CR|=DMA2D_CR_START;				//����DMA2D
	while((DMA2D->ISR&(DMA2D_FLAG_TC))==0)	//�ȴ��������
	{
		timeout++;
		if(timeout>0X1FFFFF)break;	//��ʱ�˳�
	} 
	DMA2D->IFCR|=DMA2D_FLAG_TC;		//���������ɱ�־ 	
}
/**
  * @brief  ����??��?��?��???��??���?��?��?��3?��??��?
  * @param  TextColor: ��?��???��?
  * @param  BackColor: ��?��?��?��3?��??��?
  * @retval None
  */
void LCD_SetColors(uint32_t TextColor, uint32_t BackColor) 
{
  CurrentTextColor = TextColor; 
  CurrentBackColor = BackColor;
}

/**
  * @brief ??��?�̡�?�㨦��??��?��?��???��?o����?��?��?��3?��??��?
  * @param  TextColor: ???����?��???��?��?????
  * @param  BackColor: ???����?��?��3?��??��?��?????
  * @retval None
  */
void LCD_GetColors(uint32_t *TextColor, uint32_t *BackColor)
{
  *TextColor = CurrentTextColor;
  *BackColor = CurrentBackColor;
}

/**
  * @brief  ����??��?��???��?
  * @param  Color: ��?��???��?
  * @retval None
  */
void LCD_SetTextColor(uint32_t Color)
{
  CurrentTextColor = Color;
}

/**
  * @brief  ����??��?��?��?��3?��??��?
  * @param  Color: ��?��?��?��3?��??��?
  * @retval None
  */
void LCD_SetBackColor(uint32_t Color)
{
  CurrentBackColor = Color;
}

/**
  * @brief  ����??��?��???��?(����??)
  * @param  fonts: ????��a����??��?��?��???��?
  * @retval None
  */
void LCD_SetFont(sFONT *fonts)
{
  LCD_Currentfonts = fonts;
}
/**
  * @brief  Gets the Text Font.
  * @param  None.
  * @retval the used font.
  */
sFONT *LCD_GetFont(void)
{
  return LCD_Currentfonts;
}


/**
  * @brief  Draws a character on LCD.
  * @param  Xpos: the Line where to display the character shape.
  * @param  Ypos: start column address.
  * @param  c: pointer to the character data.
  * @retval None
  */
void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0, xpos =0;
  uint32_t  Xaddress = 0;
  
  xpos = Xpos*LCD_PIXEL_WIDTH*3;
  Xaddress += Ypos;
  
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {
    
    for(counter = 0; counter < LCD_Currentfonts->Width; counter++)
    {
          
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> counter)) == 0x00) &&(LCD_Currentfonts->Width <= 12))||
        (((c[index] & (0x1 << counter)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))
      {
        *(__IO uint16_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos) = (0x00FFFF & CurrentBackColor);        //GB
        *(__IO uint8_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos+2) = (0xFF0000 & CurrentBackColor) >> 16; //R
      }
      else
      {
        *(__IO uint16_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos) = (0x00FFFF & CurrentTextColor);        //GB
        *(__IO uint8_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos+2) = (0xFF0000 & CurrentTextColor) >> 16; //R
      }
      Xaddress++;
    }
      Xaddress += (LCD_PIXEL_WIDTH - LCD_Currentfonts->Width);
  }
}
/**
  * @brief  ��ʾһ�����ص�
  * @param  x: ���ص��x����
  * @param  y: ���ص��y����
  * @retval None
  */
void PutPixel(int16_t x, int16_t y)
{ 
  if(x < 0 || x > LCD_PIXEL_WIDTH || y < 0 || y > LCD_PIXEL_HEIGHT)
  {
    return;  
  }
#if 0
 LCD_DrawLine(x, y, 1, LCD_DIR_HORIZONTAL);
#else /*����ֱ�����Ч�ʹ��ƻ�ߵ�*/
 {
	  /*RGB888*/
	  uint32_t  Xaddress =0;
    Xaddress =  CurrentFrameBuffer + 3*(LCD_PIXEL_WIDTH*y + x);
	  *(__IO uint16_t*) Xaddress= (0x00FFFF & CurrentTextColor);        //GB
  	*(__IO uint8_t*)( Xaddress+2)= (0xFF0000 & CurrentTextColor) >> 16; //R
  }
#endif
}
/**
  * @brief  Displays one character (16dots width, 24dots height).
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..29
  * @param  Column: start column address.
  * @param  Ascii: character ascii code, must be between 0x20 and 0x7E.
  * @retval None
  */
void LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii)
{
  Ascii -= 32;

  LCD_DrawChar(Line, Column, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}
/**
  * @brief  Clears the selected line.
  * @param  Line: the Line to be cleared.
  *   This parameter can be one of the following values:
  *     @arg LCD_LINE_x: where x can be: 0..13 if LCD_Currentfonts is Font16x24
  *                                      0..26 if LCD_Currentfonts is Font12x12 or Font8x12
  *                                      0..39 if LCD_Currentfonts is Font8x8
  * @retval None
  */
void LCD_ClearLine(uint16_t Line)
{
  uint16_t refcolumn = 0;
  /* Send the string character by character on lCD */
  while ((refcolumn < LCD_PIXEL_WIDTH) && (((refcolumn + LCD_Currentfonts->Width)& 0xFFFF) >= LCD_Currentfonts->Width))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, ' ');
    /* Decrement the column position by 16 */
    refcolumn += LCD_Currentfonts->Width;
  }
}
/**
  * @brief  Displays a maximum of 20 char on the LCD.
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @param  *ptr: pointer to string to display on LCD.
  * @retval None
  */
void LCD_DisplayStringLine(uint16_t Line, uint8_t *ptr)
{  
  uint16_t refcolumn = 0;
  /* Send the string character by character on lCD */
  while ((refcolumn < LCD_PIXEL_WIDTH) && ((*ptr != 0) & (((refcolumn + LCD_Currentfonts->Width) & 0xFFFF) >= LCD_Currentfonts->Width)))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, *ptr);
    /* Decrement the column position by width */
    refcolumn += LCD_Currentfonts->Width;
    /* Point on the next character */
    ptr++;
  }
}
/**
  * @brief  Displays an uni-line (between two points).
  * @param  x1: specifies the point 1 x position.
  * @param  y1: specifies the point 1 y position.
  * @param  x2: specifies the point 2 x position.
  * @param  y2: specifies the point 2 y position.
  * @retval None
  */
void LCD_DrawUniLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
  curpixel = 0;
  
  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */
  
  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    PutPixel(x, y);             /* Draw the current pixel */
    num += numadd;              /* Increase the numerator by the top of the fraction */
    if (num >= den)             /* Check if numerator >= denominator */
    {
      num -= den;               /* Calculate the new numerator value */
      x += xinc1;               /* Change the x as appropriate */
      y += yinc1;               /* Change the y as appropriate */
    }
    x += xinc2;                 /* Change the x as appropriate */
    y += yinc2;                 /* Change the y as appropriate */
  }
}
/**
  * @brief ��ʾһ��ֱ��
  * @param Xpos: ֱ������x����
  * @param Ypos: ֱ������y����
  * @param Length: ֱ�ߵĳ���
  * @param Direction: ֱ�ߵķ��򣬿�����LCD_DIR_HORIZONTAL(ˮƽ����) LCD_DIR_VERTICAL(��ֱ����).
  * @retval None
  */
void LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction)
{
	uint32_t timeout=0; 
	uint32_t  Xaddress = 0;
		/*����Ŀ���ַ*/
  Xaddress = CurrentFrameBuffer + 3*(LCD_PIXEL_WIDTH*Ypos + Xpos);
		/*ˮƽ����*/
	if(Direction == LCD_DIR_HORIZONTAL)
  { 
		hdma2d.Init.OutputOffset = 0x0;
	}
	else /*��ֱ����*/
  { 
		hdma2d.Init.OutputOffset = LCD_PIXEL_WIDTH - 1; 
	}

	/*ˮƽ����*/
	if(Direction == LCD_DIR_HORIZONTAL)
  { 
			__HAL_RCC_DMA2D_CLK_ENABLE();	//ʹ��DM2Dʱ��
			DMA2D->CR&=~(DMA2D_CR_START);	//��ֹͣDMA2D
			DMA2D->CR=DMA2D_R2M;			//�Ĵ������洢��ģʽ
			DMA2D->OPFCCR=DMA2D_OUTPUT_RGB888;	//������ɫ��ʽ
			DMA2D->OOR=hdma2d.Init.OutputOffset;				//������ƫ�� 

			DMA2D->OMAR=Xaddress;				//����洢����ַ
			DMA2D->NLR=(1)|((Length)<<16);	//�趨�����Ĵ���
			DMA2D->OCOLR=CurrentTextColor;						//�趨�����ɫ�Ĵ��� 
			DMA2D->CR|=DMA2D_CR_START;				//����DMA2D
	}
	else /*��ֱ����*/
  {  
			__HAL_RCC_DMA2D_CLK_ENABLE();	//ʹ��DM2Dʱ��
			DMA2D->CR&=~(DMA2D_CR_START);	//��ֹͣDMA2D
			DMA2D->CR=DMA2D_R2M;			//�Ĵ������洢��ģʽ
			DMA2D->OPFCCR=DMA2D_OUTPUT_RGB888;	//������ɫ��ʽ
			DMA2D->OOR=hdma2d.Init.OutputOffset;				//������ƫ�� 

			DMA2D->OMAR=Xaddress;				//����洢����ַ
			DMA2D->NLR=(Length)|((1)<<16);	//�趨�����Ĵ���
			DMA2D->OCOLR=CurrentTextColor;						//�趨�����ɫ�Ĵ��� 
			DMA2D->CR|=DMA2D_CR_START;				//����DMA2D
	}

	while((DMA2D->ISR&(DMA2D_FLAG_TC))==0)	//�ȴ��������
	{
		timeout++;
		if(timeout>0X1FFFFF)break;	//��ʱ�˳�
	} 
	DMA2D->IFCR|=DMA2D_FLAG_TC;		//���������ɱ�־ 
}
/**
  * @brief  Draw a circle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Radius: radius of the circle.
  * @retval None
  */
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  
    int x = -Radius, y = 0, err = 2-2*Radius, e2;
    do {
      

        *(__IO uint16_t*) (CurrentFrameBuffer + (3*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos+y))))= (0x00FFFF & CurrentTextColor);       //GB
        *(__IO uint8_t*)(CurrentFrameBuffer + (3*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos+y))+2))= (0xFF0000 & CurrentTextColor) >> 16; //R
      
        *(__IO uint16_t*) (CurrentFrameBuffer + (3*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos+y)))) = (0x00FFFF & CurrentTextColor);      //GB
        *(__IO uint8_t*)(CurrentFrameBuffer + (3*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos+y))+2))= (0xFF0000 & CurrentTextColor) >> 16; //R
      
        *(__IO uint16_t*) (CurrentFrameBuffer + (3*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos-y)))) = (0x00FFFF & CurrentTextColor);      //GB
        *(__IO uint8_t*)(CurrentFrameBuffer + (3*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos-y))+2))= (0xFF0000 & CurrentTextColor) >> 16; //R
      
        *(__IO uint16_t*) (CurrentFrameBuffer + (3*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos-y)))) = (0x00FFFF & CurrentTextColor);      //GB
        *(__IO uint8_t*)(CurrentFrameBuffer + (3*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos-y))+2))= (0xFF0000 & CurrentTextColor) >> 16; //R


        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    }
    while (x <= 0);
    
}
/**
  * @brief  Displays a full circle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Radius
  * @retval None
  */
void LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  D;    /* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (Radius << 1);
  
  CurX = 0;
  CurY = Radius;
  
  while (CurX <= CurY)
  {
    if(CurY > 0) 
    {
      LCD_DrawLine(Xpos - CurX, Ypos - CurY, 2*CurY, LCD_DIR_VERTICAL);
      LCD_DrawLine(Xpos + CurX, Ypos - CurY, 2*CurY, LCD_DIR_VERTICAL);
    }
    
    if(CurX > 0) 
    {
      LCD_DrawLine(Xpos - CurY, Ypos - CurX, 2*CurX, LCD_DIR_VERTICAL);
      LCD_DrawLine(Xpos + CurY, Ypos - CurX, 2*CurX, LCD_DIR_VERTICAL);
    }
    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
  
  LCD_DrawCircle(Xpos, Ypos, Radius);  
}
/**
  * @brief  ����ʵ�ľ���
  * @param  Xpos: ��ʼX����
  * @param  Ypos: ��ʼY����
  * @param  Height: ���θ�
  * @param  Width: ���ο�
  * @retval None
  */
void LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
	uint32_t timeout=0; 
	uint32_t  Xaddress = 0;
		/*����Ŀ���ַ*/
  Xaddress = CurrentFrameBuffer + 3*(LCD_PIXEL_WIDTH*Ypos + Xpos);
		
	__HAL_RCC_DMA2D_CLK_ENABLE();	//ʹ��DM2Dʱ��
	DMA2D->CR&=~(DMA2D_CR_START);	//��ֹͣDMA2D
	DMA2D->CR=DMA2D_R2M;			//�Ĵ������洢��ģʽ
	DMA2D->OPFCCR=DMA2D_OUTPUT_RGB888;	//������ɫ��ʽ
	DMA2D->OOR=(LCD_PIXEL_WIDTH - Width);;				//������ƫ�� 

	DMA2D->OMAR=Xaddress;				//����洢����ַ
	DMA2D->NLR=(Height)|((Width)<<16);	//�趨�����Ĵ���
	DMA2D->OCOLR=CurrentTextColor;						//�趨�����ɫ�Ĵ��� 
	DMA2D->CR|=DMA2D_CR_START;				//����DMA2D
	while((DMA2D->ISR&(DMA2D_FLAG_TC))==0)	//�ȴ��������
	{
		timeout++;
		if(timeout>0X1FFFFF)break;	//��ʱ�˳�
	} 
	DMA2D->IFCR|=DMA2D_FLAG_TC;		//���������ɱ�־ 
}
/**
  * @brief  Displays a rectangle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Height: display rectangle height, can be a value from 0 to 320.
  * @param  Width: display rectangle width, can be a value from 0 to 240.
  * @retval None
  */
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  /* draw horizontal lines */
  LCD_DrawLine(Xpos, Ypos, Width, LCD_DIR_HORIZONTAL);
  LCD_DrawLine(Xpos, (Ypos+ Height), Width, LCD_DIR_HORIZONTAL);
  
  /* draw vertical lines */
  LCD_DrawLine(Xpos, Ypos, Height, LCD_DIR_VERTICAL);
  LCD_DrawLine((Xpos + Width), Ypos, Height, LCD_DIR_VERTICAL);
}
/**
 * @brief  ������ģ�����ź����ģ��1�����ص���8������λ����ʾ
										0x01��ʾ�ʼ���0x00��ʾ�հ���
 * @param  in_width ��ԭʼ�ַ����
 * @param  in_heig ��ԭʼ�ַ��߶�
 * @param  out_width �����ź���ַ����
 * @param  out_heig�����ź���ַ��߶�
 * @param  in_ptr ���ֿ�����ָ��	ע�⣺1pixel 1bit
 * @param  out_ptr �����ź���ַ����ָ�� ע��: 1pixel 8bit
 *		out_ptrʵ����û������������ĳ���ֱ�������ȫ��ָ��zoomBuff��
 * @param  en_cn ��0ΪӢ�ģ�1Ϊ����
 * @retval ��
 */
void LCD_zoomChar(uint16_t in_width,	//ԭʼ�ַ����
									uint16_t in_heig,		//ԭʼ�ַ��߶�
									uint16_t out_width,	//���ź���ַ����
									uint16_t out_heig,	//���ź���ַ��߶�
									uint8_t *in_ptr,	//�ֿ�����ָ��	ע�⣺1pixel 1bit
									uint8_t *out_ptr, //���ź���ַ����ָ�� ע��: 1pixel 8bit
									uint8_t en_cn)		//0ΪӢ�ģ�1Ϊ����	
{
	uint8_t *pts,*ots;
	//����Դ��ģ��Ŀ����ģ��С���趨����������ӣ�����16��Ϊ�˰Ѹ�������ת�ɶ�������
	unsigned int xrIntFloat_16=(in_width<<16)/out_width+1; 
  unsigned int yrIntFloat_16=(in_heig<<16)/out_heig+1;
	
	unsigned int srcy_16=0;
	unsigned int y,x;
	uint8_t *pSrcLine;
	uint8_t tempBuff[1024] = {0};
	uint32_t			uChar;
	uint16_t			charBit = in_width / 8;
	uint16_t			Bitdiff = 32 - in_width;
	
	//�������Ƿ�Ϸ�
	if(in_width >= 32) return;												//�ֿⲻ������32����
	if(in_width * in_heig == 0) return;	
	if(in_width * in_heig >= 1024 ) return; 					//����������� 32*32
	
	if(out_width * out_heig == 0) return;	
	if(out_width * out_heig >= ZOOMMAXBUFF ) return; //����������� 128*128
	pts = (uint8_t*)&tempBuff;
	
	//Ϊ�������㣬�ֿ��������1 pixel 1bit ӳ�䵽1pixel 8bit
	//0x01��ʾ�ʼ���0x00��ʾ�հ���
	if(en_cn == 0x00)//Ӣ��
	{
		//������16 * 24�ֿ���Ϊ���ԣ�������С���ֿ����и������д�������ӳ��Ϳ���
		//Ӣ�ĺ������ֿ����±߽粻�ԣ����ڴ˴ε�������Ҫע��tempBuff��ֹ���
			pts+=in_width*4;
			for(y=0;y<in_heig;y++)	
			{
				uChar = *(uint32_t *)(in_ptr + y * charBit) >> Bitdiff;
				for(x=0;x<in_width;x++)
					{
						*pts++ = (uChar >> x) & 0x01;
					}
			}		
	}
	else //����
	{
			for(y=0;y<in_heig;y++)	
			{
				/*Դ��ģ����*/
				uChar = in_ptr [ y * 3 ];
				uChar = ( uChar << 8 );
				uChar |= in_ptr [ y * 3 + 1 ];
				uChar = ( uChar << 8 );
				uChar |= in_ptr [ y * 3 + 2];
				/*ӳ��*/
				for(x=0;x<in_width;x++)
					{
						if(((uChar << x) & 0x800000) == 0x800000)
							*pts++ = 0x01;
						else
							*pts++ = 0x00;
					}
			}		
	}

	//zoom����
	pts = (uint8_t*)&tempBuff;	//ӳ����Դ����ָ��
	ots = (uint8_t*)&zoomBuff;	//������ݵ�ָ��
	for (y=0;y<out_heig;y++)	/*�б���*/
    {
				unsigned int srcx_16=0;
        pSrcLine=pts+in_width*(srcy_16>>16);				
        for (x=0;x<out_width;x++) /*�������ر���*/
        {
            ots[x]=pSrcLine[srcx_16>>16]; //��Դ��ģ���ݸ��Ƶ�Ŀ��ָ����
            srcx_16+=xrIntFloat_16;			//������ƫ��Դ���ص�
        }
        srcy_16+=yrIntFloat_16;				  //������ƫ��Դ���ص�
        ots+=out_width;						
    }
	/*���������ź����ģ����ֱ�Ӵ洢��ȫ��ָ��zoomBuff����*/
	out_ptr = (uint8_t*)&zoomBuff;	//out_ptrû����ȷ�������������ֱ�Ӹĳ���ȫ�ֱ���ָ�룡
	
	/*ʵ�������ʹ��out_ptr����Ҫ������һ�䣡����
		ֻ����Ϊout_ptrû��ʹ�ã��ᵼ��warning��ǿ��֢*/
	out_ptr++; 
}			
/**
 * @brief  ����ʾ������ʾһ�������ַ�
 * @param  usX �����ض�ɨ�跽�����ַ�����ʼX����
 * @param  usY �����ض�ɨ�跽�����ַ�����ʼY����
 * @param  usChar ��Ҫ��ʾ�������ַ��������룩
 * @retval ��
 */ 
void LCD_DispChar_CH ( uint16_t usX, uint16_t usY, uint16_t usChar)
{
	uint8_t ucPage, ucColumn;
	uint8_t ucBuffer [ 24*24/8 ];	

  uint32_t usTemp; 	

	uint32_t  xpos =0;
  uint32_t  Xaddress = 0;
  
	/*xpos��ʾ��ǰ�е��Դ�ƫ��λ��*/
  xpos = usX*LCD_PIXEL_WIDTH*3;
	
	/*Xaddress��ʾ���ص�*/
  Xaddress += usY;
	   
  macGetGBKCode ( ucBuffer, usChar );	//ȡ��ģ����
	
	/*ucPage��ʾ��ǰ����*/
	for ( ucPage = 0; ucPage < macHEIGHT_CH_CHAR; ucPage ++ )
	{
    /* ȡ��3���ֽڵ����ݣ���lcd�ϼ���һ�����ֵ�һ�� */
		usTemp = ucBuffer [ ucPage * 3 ];
		usTemp = ( usTemp << 8 );
		usTemp |= ucBuffer [ ucPage * 3 + 1 ];
		usTemp = ( usTemp << 8 );
		usTemp |= ucBuffer [ ucPage * 3 + 2];
	
		
		for ( ucColumn = 0; ucColumn < macWIDTH_CH_CHAR; ucColumn ++ ) 
		{			
			if ( usTemp & ( 0x01 << 23 ) )  //��λ��ǰ 				
			{
				//����ɫ
			  *(__IO uint16_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos) = (0x00FFFF & CurrentTextColor);        //GB
        *(__IO uint8_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos+2) = (0xFF0000 & CurrentTextColor) >> 16; //R

			}				
			else	
			{
				//����ɫ
				*(__IO uint16_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos) = (0x00FFFF & CurrentBackColor);        //GB
        *(__IO uint8_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos+2) = (0xFF0000 & CurrentBackColor) >> 16; //R

			}	
			/*ָ��ǰ�е���һ����*/	
			Xaddress++;			
			usTemp <<= 1;
			
		}
		/*��ʾ��һ��*/
		/*ָ���ַ���ʾ������һ�еĵ�һ�����ص�*/
		Xaddress += (LCD_PIXEL_WIDTH - macWIDTH_CH_CHAR);
		
	}
}
/**
 * @brief  �������ź����ģ��ʾ�ַ�
 * @param  Xpos ���ַ���ʾλ��x
 * @param  Ypos ���ַ���ʾλ��y
 * @param  Font_width ���ַ����
 * @param  Font_Heig���ַ��߶�
 * @param  c ��Ҫ��ʾ����ģ����
 * @param  DrawModel ���Ƿ�ɫ��ʾ 
 * @retval ��
 */
void LCD_DrawChar_Ex(uint16_t Xpos, //�ַ���ʾλ��x
												uint16_t Ypos, //�ַ���ʾλ��y
												uint16_t Font_width, //�ַ����
												uint16_t Font_Heig,  //�ַ��߶� 
												uint8_t *c,						//��ģ����
												uint16_t DrawModel)		//�Ƿ�ɫ��ʾ
{
  uint32_t index = 0, counter = 0, xpos =0;
  uint32_t  Xaddress = 0;
  
  xpos = Xpos*LCD_PIXEL_WIDTH*3;
  Xaddress += Ypos;
  
  for(index = 0; index < Font_Heig; index++)
  {
    
    for(counter = 0; counter < Font_width; counter++)
    {
      if(*c++ == DrawModel)	//������ģ����ɫ���þ�����ʾ������ɫ
      {
        *(__IO uint16_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos) = (0x00FFFF & CurrentBackColor);        //GB
        *(__IO uint8_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos+2) = (0xFF0000 & CurrentBackColor) >> 16; //R
      }
      else
      {
        *(__IO uint16_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos) = (0x00FFFF & CurrentTextColor);        //GB
        *(__IO uint8_t*)(CurrentFrameBuffer + (3*Xaddress) + xpos+2) = (0xFF0000 & CurrentTextColor) >> 16; //R
      }
      Xaddress++;
    }
      Xaddress += (LCD_PIXEL_WIDTH - Font_width);
  }
}

/**
 * @brief  ����ʾ������ʾ��Ӣ���ַ���,����Һ�����ʱ���Զ����С�
					 ��Ӣ�Ļ���ʾʱ�����Ӣ����������ΪFont16x24��ʽ
 * @param  Line ����(Ҳ�����Ϊy����)
 * @param  Column ���У�Ҳ�����Ϊx���꣩
 * @param  pStr ��Ҫ��ʾ���ַ������׵�ַ
 * @retval ��
 */
void LCD_DispString_EN_CH( uint16_t Line, uint16_t Column, const uint8_t * pStr )
{
	uint16_t usCh;
	
	
	while( * pStr != '\0' )
	{
		if ( * pStr <= 126 )	           	//Ӣ���ַ�
		{
	
			/*�Զ�����*/
			if ( ( Column + LCD_Currentfonts->Width ) > LCD_PIXEL_WIDTH )
			{
				Column = 0;
				Line += LCD_Currentfonts->Height;
			}
			
			if ( ( Line + LCD_Currentfonts->Height ) > LCD_PIXEL_HEIGHT )
			{
				Column = 0;
				Line = 0;
			}			
					
			LCD_DisplayChar(Line,Column,*pStr);
			
			Column += LCD_Currentfonts->Width;
		
		  pStr ++;

		}
		
		else	                            //�����ַ�
		{
			if ( ( Column + macWIDTH_CH_CHAR ) > LCD_PIXEL_WIDTH )
			{
				Column = 0;
				Line += macHEIGHT_CH_CHAR;
			}
			
			if ( ( Line + macHEIGHT_CH_CHAR ) > LCD_PIXEL_HEIGHT )
			{
				Column = 0;
				Line = 0;
			}	
			
			/*һ���������ֽ�*/
			usCh = * ( uint16_t * ) pStr;				
			usCh = ( usCh << 8 ) + ( usCh >> 8 );		

			LCD_DispChar_CH (Line,Column, usCh);
			
			Column += macWIDTH_CH_CHAR;
			
			pStr += 2;           //һ�����������ֽ� 
		
    }
		
  }
	
	
}  

/**
  * @brief  ��ʾһ���ַ���������Һ����ȣ����Զ����С�
						��Ӣ����ʱ�����Ӣ����������ΪFont16x24��ʽ
  * @param  Line: Ҫ��ʾ���б��LINE(0) - LINE(N)
  * @param  *ptr: Ҫ��ʾ���ַ���ָ��
  * @retval None
  */
void LCD_DisplayStringLine_EN_CH(uint16_t Line, uint8_t *ptr)
{  
  uint16_t refcolumn = 0;
  /* Send the string character by character on lCD */
  while ((refcolumn < LCD_PIXEL_WIDTH) && ((*ptr != 0) & (((refcolumn + LCD_Currentfonts->Width) & 0xFFFF) >= LCD_Currentfonts->Width)))
  {
    /* Display one character on LCD */
		if ( * ptr <= 126 )	           	//Ӣ���ַ�
		{
					
			LCD_DisplayChar(Line, refcolumn, *ptr);
			/* Decrement the column position by width */
			refcolumn += LCD_Currentfonts->Width;
			/* Point on the next character */
			ptr++;
		}
		
		else	                            //�����ַ�
		{	
			uint16_t usCh;
			
			/*һ���������ֽ�*/
			usCh = * ( uint16_t * ) ptr;				
			usCh = ( usCh << 8 ) + ( usCh >> 8 );		
			
			LCD_DispChar_CH ( Line, refcolumn, usCh );
			refcolumn += macWIDTH_CH_CHAR;

			ptr += 2; 		
    }	

  }
}

/**
 * @brief  �������ź����ģ��ʾ�ַ���
 * @param  Xpos ���ַ���ʾλ��x
 * @param  Ypos ���ַ���ʾλ��y
 * @param  Font_width ���ַ���ȣ�Ӣ���ַ��ڴ˻�����/2��ע��Ϊż��
 * @param  Font_Heig���ַ��߶ȣ�ע��Ϊż��
 * @param  c ��Ҫ��ʾ���ַ���
 * @param  DrawModel ���Ƿ�ɫ��ʾ 
 * @retval ��
 */
void LCD_DisplayStringLineEx(uint16_t x, 		//�ַ���ʾλ��x
														 uint16_t y, 				//�ַ���ʾλ��y
														 uint16_t Font_width,	//Ҫ��ʾ�������ȣ�Ӣ���ַ��ڴ˻�����/2��ע��Ϊż��
														 uint16_t Font_Heig,	//Ҫ��ʾ������߶ȣ�ע��Ϊż��
														 uint8_t *ptr,					//��ʾ���ַ�����
														 uint16_t DrawModel)  //�Ƿ�ɫ��ʾ
{
	uint16_t refcolumn = x; //x����
	uint16_t Charwidth;
	uint8_t *psr;
	uint8_t Ascii;	//Ӣ��
	uint16_t usCh;  //����
	uint8_t ucBuffer [ 24*24/8 ];	
	
	while ((refcolumn < LCD_PIXEL_WIDTH) && ((*ptr != 0) & (((refcolumn + LCD_Currentfonts->Width) & 0xFFFF) >= LCD_Currentfonts->Width)))
	{
		if(*ptr > 0x80) //���������
		{
			Charwidth = Font_width;
			usCh = * ( uint16_t * ) ptr;				
			usCh = ( usCh << 8 ) + ( usCh >> 8 );
			macGetGBKCode ( ucBuffer, usCh );	//ȡ��ģ����
			//������ģ����
			LCD_zoomChar(24,24,Charwidth,Font_Heig,(uint8_t *)&ucBuffer,psr,1); 
			//��ʾ�����ַ�
			LCD_DrawChar_Ex(y,refcolumn,Charwidth,Font_Heig,(uint8_t*)&zoomBuff,DrawModel);
			refcolumn+=Charwidth;
			ptr+=2;
		}
		else
		{
				Charwidth = Font_width / 2;
				Ascii = *ptr - 32;
				//������ģ����
				LCD_zoomChar(16,24,Charwidth,Font_Heig,(uint8_t *)&LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height],psr,0);
			  //��ʾ�����ַ�
				LCD_DrawChar_Ex(y,refcolumn,Charwidth,Font_Heig,(uint8_t*)&zoomBuff,DrawModel);
				refcolumn+=Charwidth;
				ptr++;
		}
	}
}
