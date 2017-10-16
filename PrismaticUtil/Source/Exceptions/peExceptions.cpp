
#include "Exceptions\peExceptions.h"

namespace pe
{

#pragma region DllLoadFailedException

peDllLoadFailedException::peDllLoadFailedException(const char* what) :
    std::runtime_error(what){}

peDllLoadFailedException::~peDllLoadFailedException() {}

#pragma endregion

#pragma region WindowCreationFailedException

peWindowCreationFailedException::peWindowCreationFailedException(const char* what) :
    std::runtime_error(what) {}

#pragma endregion

#pragma region WinApiErrorException

char* WinErrorCodeToMsg(DWORD errorCode)
{
	static constexpr size_t BufferSize = 512;
	auto msg = static_cast<char*>(malloc(BufferSize * sizeof(TCHAR)));
	TCHAR buffer[BufferSize] = { 0 };
	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,     // It´s a system error
		nullptr,                                    // No string to be formatted needed
		errorCode,                                  // Hey Windows: Please explain this error!
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Do it in the standard language
		buffer,                                     // Put the message here
		((BufferSize - 1) * sizeof(TCHAR)),         // Number of bytes to store the message
		nullptr);

	auto size_needed = WideCharToMultiByte(CP_ACP, 0, buffer, BufferSize, nullptr, 0, nullptr, nullptr);
	PE_ASSERT(size_needed < (BufferSize * sizeof(TCHAR)));
	WideCharToMultiByte(CP_ACP, 0, buffer, BufferSize, msg, size_needed, nullptr, nullptr);
	return msg;
}

peWinApiErrorException::peWinApiErrorException(DWORD errorCode) :
    std::runtime_error(_message = WinErrorCodeToMsg(errorCode))
{	
}

peWinApiErrorException::~peWinApiErrorException()
{
    free(const_cast<char*>(_message));
    _message = nullptr;
}

#pragma endregion

#pragma region GlProcMissingException

peGlProcMissingException::peGlProcMissingException(const char* what) : std::runtime_error(what)
{
}

#pragma endregion

}