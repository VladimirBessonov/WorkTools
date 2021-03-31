// A simple program that uses LoadLibrary and 
// GetProcAddress to access myPuts from Myputs.dll. 

#include <windows.h> 
#include <stdio.h> 
#include <string>
#include <libloaderapi.h>
#include <functional>

int main() {



	HMODULE const ibaPlug = LoadLibraryExW(LR"(ibaPdaPluginInterface.dll)", nullptr, 0);

	using PointerFunction = char const* (__cdecl*)();

	PointerFunction const IPlugin = reinterpret_cast<PointerFunction>(GetProcAddress(ibaPlug, "IPlugin"));
	//class SpinSlideCalc : IPlugin {
	

	//};

	FreeLibrary(ibaPlug);

}