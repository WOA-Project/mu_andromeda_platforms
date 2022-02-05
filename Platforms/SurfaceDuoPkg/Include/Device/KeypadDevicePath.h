#ifndef __KEYPAD_DEVICE_PATH_H__
#define __KEYPAD_DEVICE_PATH_H__

typedef struct {
  USB_CLASS_DEVICE_PATH Keypad;
  EFI_DEVICE_PATH       End;
} KEYPAD_DEVICE_PATH;

#define DP_NODE_LEN(Type)                                                      \
  {                                                                            \
    (UINT8)sizeof(Type), (UINT8)(sizeof(Type) >> 8)                            \
  }

KEYPAD_DEVICE_PATH mInternalKeypadDevicePath = {
    {
        {
            MESSAGING_DEVICE_PATH,
            MSG_USB_CLASS_DP,
            DP_NODE_LEN(USB_CLASS_DEVICE_PATH),
        },
        0xFFFF, // VendorId: any
        0xFFFF, // ProductId: any
        3,      // DeviceClass: HID
        1,      // DeviceSubClass: boot
        1,      // DeviceProtocol: keyboard
    },
    {
        END_DEVICE_PATH_TYPE,
        END_ENTIRE_DEVICE_PATH_SUBTYPE,
        {sizeof(EFI_DEVICE_PATH_PROTOCOL), 0},
    },
};

#endif