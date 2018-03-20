#include "Toolkit.h"

// Путь к рабочему каталогу (может зависеть от настроек проекта)
std::string toolkit::WorkingDir()
{
	char path[MAX_PATH] = {};
	GetCurrentDirectoryA(MAX_PATH, path);
	PathAddBackslashA(path);
	return std::string(path);
}

// Путь к каталогу с исполняемым файлом (директория содержащая запущенный .exe) 
std::string toolkit::ExeDir()
{
	char path[MAX_PATH] = {};
	GetModuleFileNameA(NULL, path, MAX_PATH);
	PathRemoveFileSpecA(path);
	PathAddBackslashA(path);
	return std::string(path);
}

// Получить текущее время
std::time_t toolkit::CurrentTime()
{
	return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

// Получить время в виде отформатированной строки
// Например, для получения даты вида 2018-05-05 12:00:31 - нужно передать формат %Y-%m-%d %H:%M:%S
// Подробнее о форматах - http://www.cplusplus.com/reference/ctime/strftime/
std::string toolkit::TimeToStr(std::time_t * time, const char * format)
{
	tm timeInfo;
	localtime_s(&timeInfo, time);
	char strTime[32];
	strftime(strTime, 32, format, &timeInfo);
	return strTime;
}

// Логирование
// Пишет в файл и консоль (если она еть) строку со временем и указанным сообщением
void toolkit::LogMessage(std::string message, bool printTime)
{
	std::string result = "";

	if (printTime) {
		time_t time = toolkit::CurrentTime();
		result += toolkit::TimeToStr(&time, "[%Y-%m-%d %H:%M:%S] ");
	}

	result += message + '\n';
	std::cout << result;

	try {
		std::string filePath = toolkit::ExeDir() + "..\\" + LOG_FILENAME;

		std::fstream fs;
		fs.exceptions(std::ios::failbit | std::ios::badbit);
		fs.open(filePath.c_str(), std::ios_base::app);

		fs << result;
		fs.close();
	}
	catch (const std::exception &ex) {
		std::cout << ex.what() << std::endl;
	}
}

// То же что и LogMessage, с той разницей что перед сообщением будет добавляться слово "ERROR!"
void toolkit::LogError(std::string message, bool printTime)
{
	std::string newMessage = "ERROR! ";
	newMessage += message;
	toolkit::LogMessage(message, printTime);
}

// Конвертация из обычной string-строки в "широкую" wstring
std::wstring toolkit::StrToWide(const std::string& str, UINT codePage, DWORD dwFlags)
{
	std::wstring result;
	wchar_t* newString = new wchar_t[str.length() + 1];
	MultiByteToWideChar(codePage, dwFlags, str.c_str(), str.length() + 1, newString, str.length() + 1);
	result.append(newString);
	delete[] newString;
	return result;
}

// Конвертация из "широкой" wstring-строки в обычную string
std::string toolkit::WideToStr(const std::wstring& wstr, UINT codePage, DWORD dwFlags)
{
	std::string result;
	char* newString = new char[wstr.length() + 1];
	WideCharToMultiByte(codePage, dwFlags, wstr.c_str(), wstr.length() + 1, newString, wstr.length() + 1, NULL, FALSE);
	result.append(newString);
	delete[] newString;
	return result;
}