#ifndef PLATFORMFS_H
#define PLATFORMFS_H

#include <string>
#include "common/error.hpp"
#include "common/expected.hpp"

using namespace std;
using mender::common::error::Error;
using mender::common::error::NoError;
using mender::common::expected::Expected;
using ExpectedSize = Expected<size_t, Error>;
using ExpectedString = Expected<std::string, Error>;
using ExpectedBool = Expected<bool, Error>;

namespace mender {
namespace io {


using File = int;
using ExpectedFile = Expected<File, Error>;
using Bytes = std::vector<uint8_t>;

///
/// \brief Open
/// \param p: path
/// \param read: open for reading
/// \param write: open for writing
/// \return
///
ExpectedFile Open(const std::string &p, bool read = true, bool write = false); // todo: mode

///
/// \brief Close
/// \param f
/// \return
///
Error Close(File f);

///
/// \brief GetSize
/// \param f: file
/// \return size of the file
///
ExpectedSize GetSize(File f);

///
/// \brief Read
/// \param f: file
/// \param data: vector to store read data
/// \return number of bytes read or error on failure
///
ExpectedSize Read(File f, Bytes &data);

///
/// \brief Write
/// \param f: file
/// \param data
/// \return number of bytes written or error on failure
///
ExpectedSize Write(File f, const Bytes &data);

///
/// \brief Flush: flushes written data
/// \param f: file
/// \return
///
Error Flush(File f);

///
/// \brief SeekSet
/// \param f: file
/// \param pos: posiiton to be set on the file
/// \return
///
Error SeekSet(File f, uint64_t pos);

///
/// \brief Tell
/// \param f: file
/// \return current posiiton of the file seek
///
ExpectedSize Tell(File f);

///
/// \brief GetInputStream
/// \return SDTIN
///
File GetInputStream();

///
/// \brief IsSpecialBlockDevice
/// \param f: file
/// \return true if the file descriptor is special block device
///
ExpectedBool IsSpecialBlockDevice(File f);

///
/// \brief WriteFile: opens a file (creates if doesn't exist), writes the data and closes the file
/// \param path: path to the file
/// \param data: data that's will be written to the file
/// \return bytes written on an error
///
ExpectedSize WriteFile(const string &path, const Bytes &data);

///
/// \brief MakeTempDir
/// \param templateName: name of the new temp directory, the function will create a dir name by
///	appending 6 random characters to the given name
/// \return temp directory name or error
///
ExpectedString MakeTempDir(const string &templateName);

} // namespace io
} // namespace mender

#endif // PLATFORMFS_H
