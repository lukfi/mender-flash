#pragma once

#include "fileio.hpp"

namespace mender {

class OptimizedBlockDeviceWriter : public mender::io::FlushingWriter {
public:
	///
	/// \brief OptimizedBlockDeviceWriter
	/// \param f: file descriptor
	/// \param limit: write limit in bytes
	/// \param optimize: write only 'disrty' frames
	///
	OptimizedBlockDeviceWriter(io::File f, size_t limit = 0, bool optimize = false);
	ExpectedSize Write(const vector<uint8_t> &dst) override;

	void PrintStatistics() const;
private:
	std::shared_ptr<mender::io::FileReader> mReader;
	std::vector<uint8_t> mBuff;
	uint32_t mChunkSize {1024};
	ssize_t mLimit;
	mender::io::File mFd;

	bool mBypassWriting {false}; // for test only, to be removed
	struct Statistics
	{
		uint32_t mBlocksWritten {0};
		uint32_t mBlocksOmitted {0};
		size_t mBytesWritten {0};
	} mStatistics;
};

class OptimizedWriter
{
public:
	OptimizedWriter(io::FileReader& reader, io::FileReadWriterSeeker& writer, size_t blockSize = 1024*1024);
	common::error::Error Copy();

	void PrintStatistics() const;
private:
	size_t mBlockSize;
	io::FileReader& mReader;
	io::FileReadWriterSeeker& mReadWriter;
	size_t mWriteLimit {0};
	bool mBypassWriting {false}; // for test only, to be removed

	struct Statistics
	{
		uint32_t mBlocksWritten {0};
		uint32_t mBlocksOmitted {0};
		size_t mBytesWritten {0};
	} mStatistics;
};

} // namespace mender
