// Copyright Antony Polukhin, 2016-2017.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_STACKTRACE_DETAIL_FRAME_MSVC_IPP
#define BOOST_STACKTRACE_DETAIL_FRAME_MSVC_IPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
#   pragma once
#endif

#include "../frame.hpp"

#include <boost/core/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include "../detail/to_hex_array.hpp"
#include <windows.h>
#include "dbgeng.h"

#include <boost/detail/winapi/get_current_process.hpp>

#ifdef BOOST_MSVC
#   pragma comment(lib, "ole32.lib")
#   pragma comment(lib, "Dbgeng.lib")
#endif

#ifdef BOOST_WINDOWS
#   include "../detail/safe_dump_win.ipp"
#else
#   include "../detail/safe_dump_posix.ipp"
#endif

#if defined(DEFINE_GUID) && !defined(BOOST_MSVC)
    DEFINE_GUID(IID_IDebugClient,0x27fe5639,0x8407,0x4f47,0x83,0x64,0xee,0x11,0x8f,0xb0,0x8a,0xc8);
    DEFINE_GUID(IID_IDebugControl,0x5182e668,0x105e,0x416e,0xad,0x92,0x24,0xef,0x80,0x04,0x24,0xba);
    DEFINE_GUID(IID_IDebugSymbols,0x8c31e98c,0x983a,0x48a5,0x90,0x16,0x6f,0xe5,0xd6,0x67,0xa9,0x50);
#endif

// Testing. Remove later
//#   define __uuidof(x) ::IID_ ## x

namespace boost { namespace stacktrace { namespace detail {

std::size_t this_thread_frames::collect(void** memory, std::size_t size, std::size_t skip) BOOST_NOEXCEPT {
    return ::CaptureStackBackTrace(
        skip + 2,
        static_cast<boost::detail::winapi::ULONG_>(size),
        memory,
        0
    );
}

class com_global_initer: boost::noncopyable {
public:
    com_global_initer() BOOST_NOEXCEPT {
        // We do not care about the result of the function call: any result is OK for us.
        ::CoInitializeEx(0, COINIT_MULTITHREADED);
    }
    ~com_global_initer() BOOST_NOEXCEPT {
        ::CoUninitialize();
    }
};


template <class T>
class com_holder: boost::noncopyable {
    T* holder_;

public:
    com_holder(const com_global_initer&) BOOST_NOEXCEPT
        : holder_(0)
    {}

    T* operator->() const BOOST_NOEXCEPT {
        return holder_;
    }

    void** to_void_ptr_ptr() BOOST_NOEXCEPT {
        return reinterpret_cast<void**>(&holder_);
    }

    bool is_inited() const BOOST_NOEXCEPT {
        return !!holder_;
    }

    ~com_holder() BOOST_NOEXCEPT {
        if (holder_) {
            holder_->Release();
        }
    }
};


inline bool try_init_com(com_holder<::IDebugSymbols>& idebug, const com_global_initer& com) BOOST_NOEXCEPT {
    com_holder<::IDebugClient> iclient(com);
    ::DebugCreate(__uuidof(IDebugClient), iclient.to_void_ptr_ptr());

    com_holder<::IDebugControl> icontrol(com);
    iclient->QueryInterface(__uuidof(IDebugControl), icontrol.to_void_ptr_ptr());

    const bool res1 = (S_OK == iclient->AttachProcess(
        0,
        ::GetCurrentProcessId(),
        DEBUG_ATTACH_NONINVASIVE | DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND)
    );
    if (!res1) {
        return false;
    }

    const bool res2 = (S_OK == icontrol->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE));
    if (!res2) {
        return false;
    }

    const bool res = (S_OK == iclient->QueryInterface(__uuidof(IDebugSymbols), idebug.to_void_ptr_ptr()));
    if (!res) {
        return false;
    }

    return true;
}

inline std::string get_name_impl(const com_holder<::IDebugSymbols>& idebug, const void* addr, std::string* module_name = 0) {
    std::string result;
    const ULONG64 offset = reinterpret_cast<ULONG64>(addr);

    char name[256];
    name[0] = '\0';
    ULONG size = 0;
    bool res = (S_OK == idebug->GetNameByOffset(
        offset,
        name,
        sizeof(name),
        &size,
        0
    ));

    if (!res && size != 0) {
        result.resize(size);
        res = (S_OK == idebug->GetNameByOffset(
            offset,
            &result[0],
            static_cast<ULONG>(result.size()),
            &size,
            0
        ));
    } else if (res) {
        result = name;
    }

    if (!res) {
        result.clear();
        return result;
    }

    const std::size_t delimiter = result.find_first_of('!');
    if (delimiter == std::string::npos) {
        return result;
    }
    
    if (module_name) {
        *module_name = result.substr(0, delimiter);
    }
        
    result = result.substr(delimiter + 1);

    return result;
}


inline std::pair<std::string, std::size_t> get_source_file_line_impl(const com_holder<::IDebugSymbols>& idebug, const void* addr) {
    std::pair<std::string, std::size_t> result;
    const ULONG64 offset = reinterpret_cast<ULONG64>(addr);

    char name[256];
    name[0] = 0;
    ULONG size = 0;
    ULONG line_num = 0;
    bool res = (S_OK == idebug->GetLineByOffset(
        offset,
        &line_num,
        name,
        sizeof(name),
        &size,
        0
    ));

    if (res) {
        result.first = name;
        result.second = line_num;
        return result;
    }

    if (!res && size == 0) {
        return result;
    }

    result.first.resize(size);
    res = (S_OK == idebug->GetLineByOffset(
        offset,
        &line_num,
        &result.first[0],
        static_cast<ULONG>(result.first.size()),
        &size,
        0
    ));
    result.second = line_num;

    if (!res) {
        result.first.clear();
        result.second = 0;
    }

    return result;
}

inline void to_string_impl(const com_holder<::IDebugSymbols>& idebug, const void* addr, std::string& res) {
    std::string module_name;
    std::string name = boost::stacktrace::detail::get_name_impl(idebug, addr, &module_name);
    if (!name.empty()) {
        res += name;
    } else {
        res += to_hex_array(addr).data();
    }
    
    std::pair<std::string, std::size_t> source_line
        = boost::stacktrace::detail::get_source_file_line_impl(idebug, addr);
    if (!source_line.first.empty() && source_line.second) {
        res += " at ";
        res += source_line.first;
        res += ':';
        res += boost::lexical_cast<boost::array<char, 40> >(source_line.second).data();
    } else if (!module_name.empty()) {
        res += " in ";
        res += module_name;
    }
}

std::string to_string(const frame* frames, std::size_t size) {
    boost::stacktrace::detail::com_global_initer com_guard;
    boost::stacktrace::detail::com_holder<::IDebugSymbols> idebug(com_guard);
    if (!boost::stacktrace::detail::try_init_com(idebug, com_guard)) {
        return std::string();
    }

    std::string res;
    res.reserve(64 * size);
    for (std::size_t i = 0; i < size; ++i) {
        if (i < 10) {
            res += ' ';
        }
        res += boost::lexical_cast<boost::array<char, 40> >(i).data();
        res += '#';
        res += ' ';
        to_string_impl(idebug, frames[i].address(), res);
        res += '\n';
    }

    return res;
}

} // namespace detail

std::string frame::name() const {
    boost::stacktrace::detail::com_global_initer com_guard;
    boost::stacktrace::detail::com_holder<::IDebugSymbols> idebug(com_guard);
    if (!boost::stacktrace::detail::try_init_com(idebug, com_guard)) {
        return std::string();
    }
    
    return boost::stacktrace::detail::get_name_impl(idebug, addr_);
}


std::string frame::source_file() const {
    boost::stacktrace::detail::com_global_initer com_guard;
    boost::stacktrace::detail::com_holder<::IDebugSymbols> idebug(com_guard);
    if (!boost::stacktrace::detail::try_init_com(idebug, com_guard)) {
        return std::string();
    }
    return boost::stacktrace::detail::get_source_file_line_impl(idebug, addr_).first;
}

std::size_t frame::source_line() const {
    ULONG line_num = 0;

    boost::stacktrace::detail::com_global_initer com_guard;
    boost::stacktrace::detail::com_holder<::IDebugSymbols> idebug(com_guard);
    if (!boost::stacktrace::detail::try_init_com(idebug, com_guard)) {
        return 0;
    }

    const bool is_ok = (S_OK == idebug->GetLineByOffset(
        reinterpret_cast<ULONG64>(addr_),
        &line_num,
        0,
        0,
        0,
        0
    ));

    return (is_ok ? line_num : 0);
}

std::string to_string(const frame& f) {
    boost::stacktrace::detail::com_global_initer com_guard;
    boost::stacktrace::detail::com_holder<::IDebugSymbols> idebug(com_guard);
    if (!boost::stacktrace::detail::try_init_com(idebug, com_guard)) {
        return std::string();
    }

    std::string res;
    boost::stacktrace::detail::to_string_impl(idebug, f.address(), res);
    return res;
}

}} // namespace boost::stacktrace

#endif // BOOST_STACKTRACE_DETAIL_FRAME_MSVC_IPP
