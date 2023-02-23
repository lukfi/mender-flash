#pragma once

#include "fileio.hpp"

namespace mender {

class OptimizedWriter
{
public:
	OptimizedWriter(io::FileReader& reader, io::FileReadWriterSeeker& writer, size_t blockSize = 1024*1024, size_t limit = 0);
	common::error::Error Copy();

	void PrintStatistics() const;

	struct Statistics
	{
		uint32_t mBlocksWritten {0};
		uint32_t mBlocksOmitted {0};
		size_t mBytesWritten {0};
	};

	const Statistics& GetStatistics() const { return mStatistics; }
private:
	size_t mBlockSize;
	io::FileReader& mReader;
	io::FileReadWriterSeeker& mReadWriter;
	size_t mWriteLimit {0};
	bool mBypassWriting {false}; // for test only, to be removed

	Statistics mStatistics;
};

} // namespace mender
