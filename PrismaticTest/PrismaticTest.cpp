// PrismaticTest.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "peEngine.h"

#include <exception>
#include <stdio.h>

#pragma warning(disable: 4100)
int _tmain(int argc, _TCHAR* argv[])
{

   try
   {
      PrismaticEngine.Init();
      PrismaticEngine.GetUpdateSystem()->Run();
   }
   catch (std::exception& ex)
   {
      printf(ex.what());
   }

   PrismaticEngine.Shutdown();

   return 0;
}

