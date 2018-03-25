/*
* Набор общих вспомогательных инструментов для таких целей как:
* - Работа с файлами (чтение, запись и прочие манипуляции)
* - Логирование (запись логов в текстовый файл)
* - Работа со строками и текстом (конвертация и т.д)
* - Работа со временем
* - Прочее
*
* Copyright (C) 2018 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
*/

#include "Toolkit.h"

/**
* Путь к рабочему каталогу
* @return std::string - строка содержащая путь к директории
* @note нужно учитывать что рабочий каталок может зависеть от конфигурации проекта
*/
std::string toolkit::WorkingDir()
{
	char path[MAX_PATH] = {};
	GetCurrentDirectoryA(MAX_PATH, path);
	PathAddBackslashA(path);
	return std::string(path);
}

/**
* Путь к каталогу с исполняемым файлом (директория содержащая запущенный .exe)
* @return std::string - строка содержащая путь к директории
*/
std::string toolkit::ExeDir()
{
	char path[MAX_PATH] = {};
	GetModuleFileNameA(NULL, path, MAX_PATH);
	PathRemoveFileSpecA(path);
	PathAddBackslashA(path);
	return std::string(path);
}

/**
* Получить текущее время
* @return std::time_t - числовое значение текущего момента системного времени
*/
std::time_t toolkit::CurrentTime()
{
	return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

/**
* Получить время в виде отформатированной строки (напр. ГГГГ-ММ-ДД ЧЧ:ММ:CC)
* @param std::time_t * time - числовое значение времени
* @param const char * format - формат в виде c-строки (напр. %Y-%m-%d %H:%M:%S)
* @return std::string - строка с отформатированным временем
* @note подробнее о форматах - http://www.cplusplus.com/reference/ctime/strftime/
*/
std::string toolkit::TimeToStr(std::time_t * time, const char * format)
{
	tm timeInfo;
	localtime_s(&timeInfo, time);
	char strTime[32];
	strftime(strTime, 32, format, &timeInfo);
	return strTime;
}

/**
* Логирование. Пишет в файл и консоль (если она еть) строку со временем и указанным сообщением
* @param std::string message - строка с соощением
* @param bool printTime - выводить ли время
*/
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

/**
* То же что и LogMessage, с той разницей что перед сообщением будет добавляться слово "ERROR!"
* @param std::string message - строка с соощением
* @param bool printTime - выводить ли время
*/
void toolkit::LogError(std::string message, bool printTime)
{
	std::string newMessage = "ERROR! ";
	newMessage += message;
	toolkit::LogMessage(message, printTime);
}

/**
* Конвертация из обычной string-строки в "широкую" wstring
* @param const std::string& str - исходная string строка
* @param UINT codePage - идентификатор поддерживаемой операционной системой "кодовй страницы"
* @param DWORD dwFlags - тип конвертации (как конвертировать простые символы в составные)
* @return std::wstring - wide (широкая) строка
* @note это обертка winapi метода MultiByteToWideChar - https://msdn.microsoft.com/en-us/library/windows/desktop/dd319072(v=vs.85).aspx
*/
std::wstring toolkit::StrToWide(const std::string& str, UINT codePage, DWORD dwFlags)
{
	std::wstring result;
	wchar_t* newString = new wchar_t[str.length() + 1];
	MultiByteToWideChar(codePage, dwFlags, str.c_str(), (int)str.length() + 1, newString, (int)str.length() + 1);
	result.append(newString);
	delete[] newString;
	return result;
}

/**
* Конвертация из "широкой" wstring-строки в обычную string
* @param const std::wstring& wstr - исходная wstring (широкая) строка
* @param UINT codePage - идентификатор поддерживаемой операционной системой "кодовй страницы"
* @param DWORD dwFlags - тип конвертации (как конвертировать составные символы в простые)
* @return std::wstring - string строка
* @note это обертка winapi метода WideCharToMultiByte - https://msdn.microsoft.com/en-us/library/windows/desktop/dd374130(v=vs.85).aspx
*/
std::string toolkit::WideToStr(const std::wstring& wstr, UINT codePage, DWORD dwFlags)
{
	std::string result;
	char* newString = new char[wstr.length() + 1];
	WideCharToMultiByte(codePage, dwFlags, wstr.c_str(), (int)wstr.length() + 1, newString, (int)wstr.length() + 1, NULL, FALSE);
	result.append(newString);
	delete[] newString;
	return result;
}

/**
* Загрузка бинарных данных из файла
* @param const std::string &path - путь к файлу
* @param char** pData - указатель на указатель на данные, память для которых будет аллоцирована
* @param size_t * size - указатель на параметр размера (размер будет получен при загрузке)
* @return bool - состояние загрузки (удалось или нет)
*/
bool toolkit::LoadBytesFromFile(const std::string &path, char** pData, size_t * size)
{
	// Открытие файла в режиме бинарного чтения
	std::ifstream is(path.c_str(), std::ios::binary | std::ios::in | std::ios::ate);

	// Если файл открыт
	if (is.is_open())
	{
		*size = (size_t)is.tellg(); // Получить размер
		is.seekg(0, std::ios::beg); // Перейти в начало файла
		*pData = new char[*size];   // Аллокировать необходимое кол-во памяти
		is.read(*pData, *size);     // Прочесть файл и поместить данные в буфер
		is.close();                 // Закрыть файл

		return true;
	}

	return false;
}