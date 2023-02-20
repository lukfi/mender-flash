// Copyright 2023 Northern.tech AS
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#ifndef FILEIO_HPP
#define FILEIO_HPP
#include "common/io.hpp"
#include "common/expected.hpp"
#include "common/error.hpp"

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

ExpectedFile Open(const std::string &p, bool read = true, bool write = false); // todo: mode
Error Close(File f);

ExpectedSize GetSize(File f);
Error SeekSet(File f, uint64_t pos);
ExpectedSize Tell(File f);
File GetInputStream();

ExpectedBool IsSpecialBlockDevice(File f);

///
/// \brief WriteFile: opens a file (creates if doesn't exist), writes the data and closes the file
/// \param path: path to the file
/// \param data: data that's will be written to the file
/// \return bytes written on an error
///
ExpectedSize WriteFile(const string &path, const Bytes& data);

///
/// \brief MakeTempDir
/// \param templateName: name of the new temp directory, the function will create a dir name by
///	appending 6 random characters to the given name
/// \return temp directory name or error
///
ExpectedString MakeTempDir(const string &templateName);

class FileWriter : public common::io::Writer {
public:
	FileWriter(File f);
	virtual ExpectedSize Write(const vector<uint8_t> &dst) override;
protected:
	File mFd;
};

class FlushingWriter : public FileWriter {
public:
	FlushingWriter(File f, uint32_t flushInterval = 1);
	virtual ExpectedSize Write(const vector<uint8_t> &dst) override;

protected:
	uint32_t mFlushIntervalBytes;
	uint32_t mUnflushedBytesWritten {0};
};

class FileReader : public common::io::Reader {
public:
	FileReader(File fd);
	virtual ExpectedSize Read(vector<uint8_t> &dst) override;

protected:
	File mFd;
};


} // namespace io
} // namespace mender
#endif // FILEIO_H
