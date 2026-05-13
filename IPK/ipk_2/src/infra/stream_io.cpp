/**
 * @file stream_io.cpp
 * @brief Implementation of StreamIO.
 * @author Peter Stahl (xstahl01)
 */

#include "stream_io.hpp"
#include "exceptions.hpp"

#include <iostream>

namespace rdt::infra
{

    StreamIO StreamIO::open_input(const std::string &path)
    {
        StreamIO s;
        if (path.empty())
            return s; // stream will be resolved to std::cin in input()

        s.file_in_.emplace(path, std::ios::in | std::ios::binary);
        if (!s.file_in_->is_open())
            throw rdt::IoException("StreamIO: cannot open input file: " + path);

        return s;
    }

    StreamIO StreamIO::open_output(const std::string &path)
    {
        StreamIO s;
        if (path.empty())
            return s; // stream will be resolved to std::cout in output()

        s.file_out_.emplace(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!s.file_out_->is_open())
            throw rdt::IoException("StreamIO: cannot open output file: " + path);

        return s;
    }

    std::istream &StreamIO::input()
    {
        if (file_in_.has_value())
            return *file_in_;
        return std::cin;
    }

    std::ostream &StreamIO::output()
    {
        if (file_out_.has_value())
            return *file_out_;
        return std::cout;
    }

} // namespace rdt::infra
