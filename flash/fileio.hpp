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

namespace mender {
namespace io {

using File = int;
using ExpectedFile = Expected<File, Error>;

ExpectedFile Open(std::string &&p, bool read = true, bool write = false);
Error Close(File f);

ExpectedSize GetSize(File f);
Error SeekSet(File f, uint64_t pos);
ExpectedSize Tell(File f);
File GetInputStream();

///
/// \brief MakeTempDir
/// \param templateName: name of the new temp directory, the function will create a dir name by
///	appending 6 random characters to the given name
/// \return temp directory name or error
///
ExpectedString MakeTempDir(std::string &&templateName);

class FlushingWriter : public common::io::Writer {
public:
	FlushingWriter(File f);
	virtual ExpectedSize Write(const vector<uint8_t> &dst) override;

protected:
	uint32_t mFlushIntervalBytes {1}; // always
	uint32_t mUnflushedBytesWritten {0};
	int mFd;
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
