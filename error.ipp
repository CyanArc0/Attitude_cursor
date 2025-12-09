[[noreturn]]static void send_error(Windows::LPCTSTR message)noexcept
{
	Windows::MessageBox(NULL,message,TEXT("Error"),MB_OK|MB_ICONERROR|MB_TOPMOST);
	std::exit(EXIT_FAILURE);
}
#define OK void(0)
#define Error(_) send_error(TEXT(_))