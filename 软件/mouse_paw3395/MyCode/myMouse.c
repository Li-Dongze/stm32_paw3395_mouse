#include "myMouse.h"
#include "usbd_hid.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

extern int16_t X_Speed,Y_Speed;

extern uint8_t Left_Key_Value;
extern uint8_t Right_Key_Value;
extern uint8_t Middel_Key_Value;

extern int8_t wheel_num;

//给鼠标报文赋初值
void myMouse_init(struct mouseHID_t* mouseHID)
{
//	mouseHID->button_left = 0;
//	mouseHID->button_middel = 0;
//	mouseHID->button_right = 0;
//	
//	mouseHID->u_x_move.x_move = 0;
//	mouseHID->u_y_move.y_move = 0;
//	
//	mouseHID->wheel = 0;
	
	mouseHID->button_left = 0;
	mouseHID->button_right = 0;
	mouseHID->button_middel = 0;
	
	mouseHID->x = 0;
	mouseHID->y = 0;
	mouseHID->wheel = 0;
}

//更新鼠标报文
void myMouse_update(struct mouseHID_t* mouseHID)
{		
//	mouseHID->u_x_move.t_x_move.x_move_l = motion_burst_data[2];
//	mouseHID->u_x_move.t_x_move.x_move_h = motion_burst_data[3];
//	mouseHID->u_y_move.t_y_move.y_move_l = motion_burst_data[4];
//	mouseHID->u_y_move.t_y_move.y_move_h = motion_burst_data[5];
//	
//	mouseHID->u_x_move.x_move = mouseHID->u_x_move.t_x_move.x_move_l + (mouseHID->u_x_move.t_x_move.x_move_h << 8);
//	mouseHID->u_y_move.y_move = mouseHID->u_y_move.t_y_move.y_move_l + (mouseHID->u_y_move.t_y_move.y_move_h << 8);
	
	mouseHID->button_left = Left_Key_Value;
	mouseHID->button_right = Right_Key_Value;
	mouseHID->button_middel = Middel_Key_Value;
	
	mouseHID->x = X_Speed;
	mouseHID->y = Y_Speed;	
	
	mouseHID->wheel = wheel_num;
	
	USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t*)mouseHID,sizeof(struct mouseHID_t));
	
	wheel_num = 0x80;
}
