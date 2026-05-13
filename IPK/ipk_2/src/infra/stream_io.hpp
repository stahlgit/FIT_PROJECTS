/**
 * @file stream_io.hpp
 * @brief RAII wrapper that opens a file for reading or writing, or falls back
 *        to stdin / stdout when no path is given.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef STREAM_IO_HPP
#define STREAM_IO_HPP

#include <fstream>
#include <iosfwd>
#include <optional>
#include <string>

namespace rdt::infra
{

    /**
     * @brief Owns optional file stream and exposes it as std::istream& or std::ostream&
     *
     * @example Constructed exclusively through the two factory methods:
     *   - StreamIO::open_input(path) - file for reading, or std::cin when path is empty
     *   - StreamIO::open_output(path) - file for writing, or std::cout when path is empty
     *
     * @throws rdt::IoException  if a non-empty path cannot be opened
     */
    class StreamIO
    {
    public:
        StreamIO(const StreamIO &) = delete;
        StreamIO &operator=(const StreamIO &) = delete;
        StreamIO(StreamIO &&) = default;
        StreamIO &operator=(StreamIO &&) = default;
        ~StreamIO() = default;

        // Factory
        /**
         * @brief Open path for reading, or wrap std::cin when path is empty.
         * @param path  File path, or empty string to use standard input.
         */
        [[nodiscard]] static StreamIO open_input(const std::string &path);

        /**
         * @brief Open path for writing (truncate), or wrap std::cout when path is empty.
         * @param path  File path, or empty string to use standard output.
         */
        [[nodiscard]] static StreamIO open_output(const std::string &path);


        // Getters
        /**
         * @brief Returns the managed input stream.
         * @pre Must have been constructed via open_input().
         */
        [[nodiscard]] std::istream &input();

        /**
         * @brief Returns the managed output stream.
         * @pre Must have been constructed via open_output().
         */
        [[nodiscard]] std::ostream &output();

    private:
        StreamIO() = default;

        std::optional<std::ifstream> file_in_{};
        std::optional<std::ofstream> file_out_{};
    };

} // namespace rdt::infra

#endif // STREAM_IO_HPP
