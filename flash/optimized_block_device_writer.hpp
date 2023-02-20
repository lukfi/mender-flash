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

private:
	std::shared_ptr<mender::io::FileReader> mReader;
	std::vector<uint8_t> mBuff;
	uint32_t mChunkSize {1024};
	ssize_t mLimit;
	mender::io::File mFd;

	bool mBypassWriting {false}; // for test only, to be removed
};

} // namespace mender
