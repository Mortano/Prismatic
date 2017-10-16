
#include "SubsystemImpl\peInputSystem.h"
#include "peEngine.h"

namespace pe
{

   void peInputSystem::Init()
   {
      _mouseDelta.x = _mouseDelta.y = 0;

      memset(_keyStates, 0, MAX_KEYS * sizeof(KeyState));
      memset(_mouseStates, 0, static_cast<uint8_t>(MouseButton::NUM_MOUSEBUTTONS) * sizeof(KeyState));

      //Register raw mouse input device
      RAWINPUTDEVICE Rid;
      Rid.usUsagePage = 0x01;
      Rid.usUsage = 0x02;
      Rid.dwFlags = 0;
      Rid.hwndTarget = 0;
      BOOL r = RegisterRawInputDevices(&Rid, 1, sizeof(Rid));
      PE_ASSERT(r);
   }

   void peInputSystem::Shutdown()
   {
      _mouseDelta.x = _mouseDelta.y = 0;

      memset(_keyStates, 0, MAX_KEYS * sizeof(KeyState));
      memset(_mouseStates, 0, static_cast<uint8_t>(MouseButton::NUM_MOUSEBUTTONS) * sizeof(KeyState));
   }

   void peInputSystem::Update(double deltaTime)
   {
      _mouseDelta.x = _mouseDelta.y = 0;
      MSG msg = { 0 };
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         switch (msg.message)
         {
         case WM_QUIT:
            PrismaticEngine.GetUpdateSystem()->Stop();
            break;
         case WM_KEYDOWN: //Handle keyboard key down messages
         {
            auto keyCode = static_cast<uint8_t>( msg.wParam );
            auto lastKeyState = _keyStates[keyCode]; //Current becomes last!
            _keyStates[keyCode] = KeyState::Pressed;

            if (lastKeyState == KeyState::NotPressed)
            {
               PrismaticEngine.GetLogging()->LogInfo("key down %u\n", keyCode);
               //Raise the key down event for the current key
               _keyEvent(keyCode, KeyState::Pressed);
            }
         }
         break;
         case WM_KEYUP: //Handle keyboard key up messages
         {
            auto keyCode = static_cast<uint8_t>( msg.wParam );
            auto lastKeyState = _keyStates[keyCode]; //Current becomes last!
            _keyStates[keyCode] = KeyState::NotPressed;

            if (lastKeyState == KeyState::Pressed)
            {
               PrismaticEngine.GetLogging()->LogInfo("key up %u\n", keyCode);
               //Raise the key down event for the current key
               _keyEvent(keyCode, KeyState::NotPressed);
            }
         }
         break;
         case WM_INPUT: //Handle remaining raw input
         {
            UINT dwSize = 0;

            GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
            if (dwSize > 0)
            {
               auto buffer = static_cast<uint8_t*>(alloca(dwSize));
               UINT bytesWritten = GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, buffer, &dwSize, sizeof(RAWINPUTHEADER));
               PE_ASSERT(dwSize == bytesWritten);
               RAWINPUT* raw = (RAWINPUT*)buffer;
               if (raw->header.dwType == RIM_TYPEMOUSE)
               {
                  _mouseDelta.x = (float)raw->data.mouse.lLastX;
                  _mouseDelta.y = (float)raw->data.mouse.lLastY;
               }
            }
         }
         break;
         default:
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            break;
         }
      }
   }

   KeyState peInputSystem::GetKeyState(KeyCode_t key) const
   {
      return _keyStates[key];
   }

   peVector2f peInputSystem::GetMouseDelta() const
   {
      return _mouseDelta;
   }

   KeyState peInputSystem::GetMouseButtonState(MouseButton mouseButton) const
   {
      return _mouseStates[static_cast<uint8_t>(mouseButton)];
   }

}