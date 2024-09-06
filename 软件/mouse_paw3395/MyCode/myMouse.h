#ifndef _MYMOUSE_H_
#define _MYMOUSE_H_

#include "main.h"

//鼠标信息结构体
__packed struct mouseHID_t
{
	/*鼠标按键：
	 *	bit0: 左键	
	 *	bit1: 右键 
	 *	bit2: 中键
	 *	bit3: 自定义
	 *	...
	 *	bit7: 自定义
	 */
//	uint8_t button_left:1;
//	uint8_t button_right:1;
//	uint8_t button_middel:1;
	
//	//x轴移动值
//	union{
//		struct{
//			uint8_t x_move_l;
//			uint8_t x_move_h;
//		}t_x_move;
//		uint16_t x_move;
//	}u_x_move;
//	
//	//y轴移动值
//	union{
//		struct{
//			uint8_t y_move_l;
//			uint8_t y_move_h;
//		}t_y_move;
//		uint16_t y_move;
//	}u_y_move;
	
	uint8_t button_left:1;      //左键
	uint8_t button_right:1;     //右键
	uint8_t button_middel:1;    //中建
	
	int16_t x;                  //x轴相对位移
	int16_t y;                  //y轴相对位移
	
	int8_t wheel;               //滚轮位移大小
};

void myMouse_init(struct mouseHID_t* mouseHID);
void myMouse_update(struct mouseHID_t* mouseHID);

#endif
