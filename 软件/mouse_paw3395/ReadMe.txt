由于STM32CubeMX配置HID协议时，只能配默认值，以下为需要修改的地方





/*
 *更改HID_FS_BINTERVAL轮询时间，每1ms更新一次
 */

更改usbd_config.h文件：

原：
#define HID_FS_BINTERVAL     0xA			//10ms

改为：
#define HID_FS_BINTERVAL     0x01			//1ms







/*
 *更改usbd_hid鼠标报文，使鼠标可以发送最大6字节数据，保证x,y轴移动数据不丢包
 *
 *全部复制粘贴
 */

更改usbd_hid.h文件：

#define HID_EPIN_ADDR                 0x81U
#define HID_EPIN_SIZE                 0x06U			//最大数据为6Byte

#define USB_HID_CONFIG_DESC_SIZ       34U
#define USB_HID_DESC_SIZ              9U
#define HID_MOUSE_REPORT_DESC_SIZE    69U

#define HID_DESCRIPTOR_TYPE           0x21U
#define HID_REPORT_DESC               0x22U

#ifndef HID_HS_BINTERVAL
#define HID_HS_BINTERVAL            0x04U
#endif /* HID_HS_BINTERVAL */

#ifndef HID_FS_BINTERVAL
#define HID_FS_BINTERVAL            0x01U
#endif /* HID_FS_BINTERVAL */

#define HID_REQ_SET_PROTOCOL          0x0BU
#define HID_REQ_GET_PROTOCOL          0x03U

#define HID_REQ_SET_IDLE              0x0AU
#define HID_REQ_GET_IDLE              0x02U

#define HID_REQ_SET_REPORT            0x09U
#define HID_REQ_GET_REPORT            0x01U



更改usbd_hid.c文件：
__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE]  __ALIGN_END =
{
    0x05, 0x01,     
    0x09, 0x02,     
    0xA1, 0x01,     
    0x05, 0x09,     
    0x19, 0x01,     
    0x29, 0x05,     
    0x15, 0x00,     
    0x25, 0x01,     
    0x95, 0x05,     
    0x75, 0x01,     
    0x81, 0x02,     
    0x95, 0x01,     
    0x75, 0x03,     
    0x81, 0x03,     

    0x05, 0x01,     
    0x09, 0x30,     
    0x09, 0x31,     
    0x16, 0x00, 0x80,
    0x26, 0xFF, 0x7F,
    0x36, 0x00, 0x80,
    0x46, 0xFF, 0x7F,
    0x75, 0x10,     
    0x95, 0x02,     
    0x81, 0x06,     

    0x09, 0x38,     
    0x15, 0x81,     
    0x25, 0x7F,     
    0x35, 0x81,     
    0x45, 0x7F,     
    0x75, 0x08,     
    0x95, 0x01,     
    0x81, 0x06,     

    0xC0            
};

