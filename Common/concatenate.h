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