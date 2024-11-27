#pragma once
#include "pch.h"
// Helper function to convert a single value to a string
template<typename T>
std::wstring ToString(T&& value) {
    std::wstringstream ss;
    ss << std::boolalpha << std::forward<T>(value);
    return ss.str();
}

// Base case for the variadic function template
inline std::wstring Concatenate() {
    return L"";
}

// Variadic function template to handle multiple arguments
template<typename T, typename... Args>
std::wstring Concatenate(T&& first, Args&&... args) {
    return ToString(std::forward<T>(first)) + Concatenate(std::forward<Args>(args)...);
}

#pragma once

#include <windows.h>
#include <string>

inline std::wstring multi2wide(const std::string& str, UINT codePage = CP_THREAD_ACP)
{
	if (str.empty())
	{
		return std::wstring();
	}

	int required = ::MultiByteToWideChar(codePage, 0, str.data(), (int)str.size(), NULL, 0);
	if (0 == required)
	{
		return std::wstring();
	}

	std::wstring str2;
	str2.resize(required);

	int converted = ::MultiByteToWideChar(codePage, 0, str.data(), (int)str.size(), &str2[0], str2.capacity());
	if (0 == converted)
	{
		return std::wstring();
	}

	return str2;
}

inline std::string wide2multi(const std::wstring& str, UINT codePage = CP_THREAD_ACP)
{
	if (str.empty())
	{
		return std::string();
	}

	int required = ::WideCharToMultiByte(codePage, 0, str.data(), (int)str.size(), NULL, 0, NULL, NULL);
	if (0 == required)
	{
		return std::string();
	}

	std::string str2;
	str2.resize(required);

	int converted = ::WideCharToMultiByte(codePage, 0, str.data(), (int)str.size(), &str2[0], str2.capacity(), NULL, NULL);
	if (0 == converted)
	{
		return std::string();
	}

	return str2;
}
