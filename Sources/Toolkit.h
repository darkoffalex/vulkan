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

#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <time.h>
#include <chrono>

// Необходимо для функций получения путей к файлу
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

// Наименования лог-файла
#ifndef LOG_FILENAME
#define LOG_FILENAME "log.txt"
#endif

namespace toolkit
{
	/**
	* Путь к рабочему каталогу
	* @return std::string - строка содержащая путь к директории
	* @note нужно учитывать что рабочий каталок может зависеть от конфигурации проекта
	*/
	std::string WorkingDir();

	/**
	* Путь к каталогу с исполняемым файлом (директория содержащая запущенный .exe) 
	* @return std::string - строка содержащая путь к директории
	*/
	std::string ExeDir();

	/**
	* Получить текущее время
	* @return std::time_t - числовое значение текущего момента системного времени
	*/
	std::time_t CurrentTime();

	/**
	* Получить время в виде отформатированной строки (напр. ГГГГ-ММ-ДД ЧЧ:ММ:CC)
	* @param std::time_t * time - числовое значение времени
	* @param const char * format - формат в виде c-строки (напр. %Y-%m-%d %H:%M:%S)
	* @return std::string - строка с отформатированным временем
	* @note подробнее о форматах - http://www.cplusplus.com/reference/ctime/strftime/
	*/
	std::string TimeToStr(std::time_t * time, const char * format);

	/**
	* Логирование. Пишет в файл и консоль (если она еть) строку со временем и указанным сообщением
	* @param std::string message - строка с соощением
	* @param bool printTime - выводить ли время
	*/
	void LogMessage(std::string message, bool printTime = true);

	/**
	* То же что и LogMessage, с той разницей что перед сообщением будет добавляться слово "ERROR!"
	* @param std::string message - строка с соощением
	* @param bool printTime - выводить ли время
	*/
	void LogError(std::string message, bool printTime = true);

	/**
	* Конвертация из обычной string-строки в "широкую" wstring
	* @param const std::string& str - исходная string строка
	* @param UINT codePage - идентификатор поддерживаемой операционной системой "кодовй страницы"
	* @param DWORD dwFlags - тип конвертации (как конвертировать простые символы в составные)
	* @return std::wstring - wide (широкая) строка
	* @note это обертка winapi метода MultiByteToWideChar - https://msdn.microsoft.com/en-us/library/windows/desktop/dd319072(v=vs.85).aspx
	*/
	std::wstring StrToWide(const std::string& str, UINT codePage = CP_ACP, DWORD dwFlags = MB_PRECOMPOSED);

	/**
	* Конвертация из "широкой" wstring-строки в обычную string
	* @param const std::wstring& wstr - исходная wstring (широкая) строка
	* @param UINT codePage - идентификатор поддерживаемой операционной системой "кодовй страницы"
	* @param DWORD dwFlags - тип конвертации (как конвертировать составные символы в простые)
	* @return std::wstring - string строка
	* @note это обертка winapi метода WideCharToMultiByte - https://msdn.microsoft.com/en-us/library/windows/desktop/dd374130(v=vs.85).aspx
	*/
	std::string WideToStr(const std::wstring& wstr, UINT codePage = CP_ACP, DWORD dwFlags = WC_COMPOSITECHECK);

	/**
	* Загрузка бинарных данных из файла
	* @param const std::string &path - путь к файлу
	* @param char** pData - указатель на указатель на данные, память для которых будет аллоцирована
	* @param size_t * size - указатель на параметр размера (размер будет получен при загрузке)
	* @return bool - состояние загрузки (удалось или нет)
	*/
	bool LoadBytesFromFile(const std::string &path, char** pData, size_t * size);
};

