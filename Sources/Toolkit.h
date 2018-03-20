#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <time.h>
#include <chrono>

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

// Наименования лог-файла
#ifndef LOG_FILENAME
#define LOG_FILENAME "log.txt"
#endif

// Пространство имен с набором вспомогательных функций для таких целей, как:
// - Работа с файлами (чтение, запись и прочие манипуляции)
// - Логирование (запись логов в текстовый файл)
// - Работа со строками и текстом (конвертация и т.д)
// - Работа со временем
// - Прочее...
namespace toolkit
{
	// Путь к рабочему каталогу (может зависеть от настроек проекта)
	std::string WorkingDir();

	// Путь к каталогу с исполняемым файлом (директория содержащая запущенный .exe) 
	std::string ExeDir();

	// Получить текущее время
	std::time_t CurrentTime();

	// Получить время в виде отформатированной строки
	// Например, для получения даты вида 2018-05-05 12:00:31 - нужно передать формат %Y-%m-%d %H:%M:%S
	// Подробнее о форматах - http://www.cplusplus.com/reference/ctime/strftime/
	std::string TimeToStr(std::time_t * time, const char * format);

	// Логирование
	// Пишет в файл и консоль (если она еть) строку со временем и указанным сообщением
	void LogMessage(std::string message, bool printTime = true);

	// То же что и LogMessage, с той разницей что перед сообщением будет добавляться слово "ERROR!"
	void LogError(std::string message, bool printTime = true);

	// Конвертация из обычной string-строки в "широкую" wstring
	std::wstring StrToWide(const std::string& str, UINT codePage = CP_ACP, DWORD dwFlags = MB_PRECOMPOSED);

	// Конвертация из "широкой" wstring-строки в обычную string
	std::string WideToStr(const std::wstring& wstr, UINT codePage = CP_ACP, DWORD dwFlags = WC_COMPOSITECHECK);
};

