/**
 * @file exceptions.hpp
 * @brief Custom exception classes for the scanner application.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

// Base exception for the entire scanner application
class ScannerError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class DnsError : public ScannerError {
public:
    using ScannerError::ScannerError;
};

// For OS-level failures (socket creation, getifaddrs, sendto)
class SystemError : public ScannerError {
public:
    using ScannerError::ScannerError;
};

// For interface parsing or matching errors
class InterfaceError : public ScannerError {
public:
    using ScannerError::ScannerError;
};

// For PCAP related failures
class PcapError : public ScannerError {
public:
    using ScannerError::ScannerError;
};

#endif // EXCEPTIONS_HPP