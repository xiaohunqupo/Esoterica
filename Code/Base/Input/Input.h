#pragma once

#include "Base/TypeSystem/ReflectedType.h"

//-------------------------------------------------------------------------

namespace EE::Input
{
    enum class InputID
    {
        EE_REFLECT_ENUM

        None = -1,

        // Keyboard
        //-------------------------------------------------------------------------

        Keyboard_A = 0,
        Keyboard_B,
        Keyboard_C,
        Keyboard_D,
        Keyboard_E,
        Keyboard_F,
        Keyboard_G,
        Keyboard_H,
        Keyboard_I,
        Keyboard_J,
        Keyboard_K,
        Keyboard_L,
        Keyboard_M,
        Keyboard_N,
        Keyboard_O,
        Keyboard_P,
        Keyboard_Q,
        Keyboard_R,
        Keyboard_S,
        Keyboard_T,
        Keyboard_U,
        Keyboard_V,
        Keyboard_W,
        Keyboard_X,
        Keyboard_Y,
        Keyboard_Z,
        Keyboard_0,
        Keyboard_1,
        Keyboard_2,
        Keyboard_3,
        Keyboard_4,
        Keyboard_5,
        Keyboard_6,
        Keyboard_7,
        Keyboard_8,
        Keyboard_9,
        Keyboard_Comma,
        Keyboard_Period,
        Keyboard_ForwardSlash,
        Keyboard_SemiColon,
        Keyboard_Quote,
        Keyboard_LBracket,
        Keyboard_RBracket,
        Keyboard_BackSlash,
        Keyboard_Underscore,
        Keyboard_Minus = Keyboard_Underscore,
        Keyboard_Equals,
        Keyboard_Plus = Keyboard_Equals,
        Keyboard_Backspace,
        Keyboard_Tilde,
        Keyboard_Tab,
        Keyboard_CapsLock,
        Keyboard_Enter,
        Keyboard_Escape,
        Keyboard_Space,
        Keyboard_Left,
        Keyboard_Up,
        Keyboard_Right,
        Keyboard_Down,
        Keyboard_NumLock,
        Keyboard_Numpad0,
        Keyboard_Numpad1,
        Keyboard_Numpad2,
        Keyboard_Numpad3,
        Keyboard_Numpad4,
        Keyboard_Numpad5,
        Keyboard_Numpad6,
        Keyboard_Numpad7,
        Keyboard_Numpad8,
        Keyboard_Numpad9,
        Keyboard_NumpadEnter,
        Keyboard_NumpadMultiply,
        Keyboard_NumpadPlus,
        Keyboard_NumpadMinus,
        Keyboard_NumpadPeriod,
        Keyboard_NumpadDivide,
        Keyboard_F1,
        Keyboard_F2,
        Keyboard_F3,
        Keyboard_F4,
        Keyboard_F5,
        Keyboard_F6,
        Keyboard_F7,
        Keyboard_F8,
        Keyboard_F9,
        Keyboard_F10,
        Keyboard_F11,
        Keyboard_F12,
        Keyboard_F13,
        Keyboard_F14,
        Keyboard_F15,
        Keyboard_PrintScreen,
        Keyboard_ScrollLock,
        Keyboard_Pause,
        Keyboard_Insert,
        Keyboard_Delete,
        Keyboard_Home,
        Keyboard_End,
        Keyboard_PageUp,
        Keyboard_PageDown,
        Keyboard_Application,
        Keyboard_LShift,
        Keyboard_RShift,
        Keyboard_LCtrl,
        Keyboard_RCtrl,
        Keyboard_LAlt,
        Keyboard_RAlt,

        // Mouse
        //-------------------------------------------------------------------------

        Mouse_Button1,
        Mouse_Left = Mouse_Button1,
        Mouse_Button2,
        Mouse_Right = Mouse_Button2,
        Mouse_Button3,
        Mouse_Middle = Mouse_Button3,
        Mouse_Button4,
        Mouse_Button5,
        Mouse_Button6,
        Mouse_Button7,
        Mouse_Button8,
        Mouse_Button9,
        Mouse_WheelHorizontal,
        Mouse_WheelVertical,
        Mouse_DeltaMovementHorizontal,
        Mouse_DeltaMovementVertical,

        // Controller
        //-------------------------------------------------------------------------

        Controller_DPadUp,
        Controller_DPadDown,
        Controller_DPadLeft,
        Controller_DPadRight,
        Controller_LeftStick,
        Controller_RightStick,
        Controller_LeftShoulder,
        Controller_RightShoulder,
        Controller_System0,
        Controller_System1,
        Controller_FaceButtonDown,
        Controller_FaceButtonRight,
        Controller_FaceButtonLeft,
        Controller_FaceButtonUp,
        Controller_LeftTrigger,
        Controller_RightTrigger,
        Controller_LeftStickHorizontal,
        Controller_LeftStickVertical,
        Controller_RightStickHorizontal,
        Controller_RightStickVertical,

        //-------------------------------------------------------------------------

        NumInputs
    };

    //-------------------------------------------------------------------------

    EE_FORCE_INLINE bool IsValidInputID( InputID ID ) { return ID != InputID::None && ID != InputID::NumInputs; }

    //-------------------------------------------------------------------------

    enum class DeviceType
    {
        Unknown,
        KeyboardMouse,
        Controller
    };

    EE_BASE_API DeviceType GetDeviceTypeForInputID( InputID ID );

    //-------------------------------------------------------------------------

    enum class InputState : uint8_t
    {
        None = 0,
        Pressed,
        Held,
        Released,
    };
}