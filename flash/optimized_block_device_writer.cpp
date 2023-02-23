#include "optimized_block_device_writer.hpp"
#include <iostream>

using namespace mender;

OptimizedBlockDeviceWriter::OptimizedBlockDeviceWriter(io::File f, size_t limit, bool optimize) :
	FlushingWriter(f),
	mLimit(limit),
	mFd(f) {
	auto blockDevice = mender::io::IsSpecialBlockDevice(mFd);
	auto size = io::GetSize(mFd);

	if (blockDevice.has_value() && blockDevice.value()) {
		mBypassWriting = true;
		mLimit = size.value();
	}

	//	std::cout << "Total size of device: " << size << " bytes\n";
	if (optimize) {
		mReader = std::make_shared<mender::io::FileReader>(mFd);
	}
}

#ifdef DEBUG
#include <iostream>
#include <iomanip>
static void printBuffer(const char *buffer, size_t size) {
	const uint8_t *u8Buffer = reinterpret_cast<const uint8_t *>(buffer);
	for (size_t i = 0; i < size; i++) {
		std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) u8Buffer[i] << ' ';
	}
	std::cout << std::endl;
}
#endif

ExpectedSize OptimizedBlockDeviceWriter::Write(const vector<uint8_t> &dst) {
	auto pos = io::Tell(mFd);
	if (!pos) {
		return pos.error();
	}
	// printf("> Compare: size: %zd offset: %zd\n", dst.size(), io::Tell(mFd).value());

	mChunkSize = dst.size();

	if (mLimit && pos + mChunkSize > mLimit) {
		return Error(std::error_condition(std::errc::io_error), "Error writing beyound the limit");
	}

	bool skipWriting = false;

	if (mReader) {
		mBuff.resize(mChunkSize);
		auto r = mReader->Read(mBuff);
		if (r) {
			mBuff.resize(r.value());
			skipWriting = std::equal(dst.begin(), dst.end(), mBuff.data());
			if (skipWriting) {
				++mStatistics.mBlocksOmitted;
			}
			printf("Compare read: %ld/%ld bytes = %d\n", r.value(), dst.size(), skipWriting);
		}
#ifdef DEBUG
		if (!skipWriting) {
			printBuffer(reinterpret_cast<const char *>(dst.data()), dst.size());
			printf("\n");
			printBuffer(reinterpret_cast<const char *>(mBuff.data()), mBuff.size());
		}
#endif
	}
	if (!skipWriting && !mBypassWriting) {
		auto res = FlushingWriter::Write(dst);
		if (res) {
			++mStatistics.mBlocksWritten;
			mStatistics.mBytesWritten += res.value();
		}
		return res;
	}
	return 0;
}

void OptimizedBlockDeviceWriter::PrintStatistics() const {
	std::cout << "================ STATISTICS ================" << std::endl;
	std::cout << "Blocks written: " << mStatistics.mBlocksWritten << std::endl;
	std::cout << "Blocks omitted: " << mStatistics.mBlocksOmitted << std::endl;
	std::cout << "Bytes  written: " << mStatistics.mBytesWritten << std::endl;
	std::cout << "============================================" << std::endl;
}

OptimizedWriter::OptimizedWriter(
	io::FileReader &reader, io::FileReadWriterSeeker &writer, size_t blockSize) :
	mBlockSize(blockSize),
	mReader(reader),
	mReadWriter(writer) {
}

Error OptimizedWriter::Copy() {
	io::Bytes rv;
	rv.resize(mBlockSize);
	io::Bytes wv;
	wv.resize(mBlockSize);

	while (true) {
		if (rv.size() != mBlockSize) {
			rv.resize(mBlockSize);
		}

		auto pos = mReader.Tell();
		if (!pos) {
			return pos.error();
		}
		auto position = pos.value();

		auto result = mReader.Read(rv);
		if (!result) {
			return result.error();
		} else if (result.value() == 0) {
			return NoError;
		} else if (result.value() > rv.size()) {
			return mender::common::error::MakeError(
				mender::common::error::ProgrammingError,
				"Read returned more bytes than requested. This is a bug in the Read function.");
		}

		auto readBytes = result.value();

		if (mWriteLimit && (position + readBytes > mWriteLimit)) {
			return Error(
				std::error_condition(std::errc::io_error), "Error writing beyound the limit");
		}

		if (readBytes != rv.size()) {
			// Because we only ever resize down, this should be very cheap. Resizing
			// back up to capacity below is then also cheap.
			rv.resize(readBytes);
		}

		if (wv.size() != readBytes) {
			wv.resize(readBytes);
		}

		bool skipWriting = false;

		if (NoError != mReadWriter.SeekSet(position)) {
			return Error(
				std::error_condition(std::errc::io_error),
				"Failed to set seek on the destination file");
		}

		auto readResult = mReadWriter.Read(wv);
		if (readResult) {
			wv.resize(readResult.value());
			skipWriting = std::equal(rv.begin(), rv.end(), wv.data());
			if (skipWriting) {
				++mStatistics.mBlocksOmitted;
			}
			printf(
				"Compare read: %ld/%ld bytes = %d\n", readResult.value(), wv.size(), skipWriting);
		}

		if (!skipWriting && !mBypassWriting) {
			mReadWriter.SeekSet(position);
			auto res = mReadWriter.Write(rv);
			if (res) {
				++mStatistics.mBlocksWritten;
				mStatistics.mBytesWritten += res.value();
			} else if (result.value() == 0) {
				return Error(
					std::error_condition(std::errc::io_error), "Zero write when copying data");
			} else if (result.value() != rv.size()) {
				return Error(
					std::error_condition(std::errc::io_error), "Short write when copying data");
			}
		}
	}
	return NoError;
}

void OptimizedWriter::PrintStatistics() const {
	std::cout << "================ STATISTICS ================" << std::endl;
	std::cout << "Blocks written: " << mStatistics.mBlocksWritten << std::endl;
	std::cout << "Blocks omitted: " << mStatistics.mBlocksOmitted << std::endl;
	std::cout << "Bytes  written: " << mStatistics.mBytesWritten << std::endl;
	std::cout << "============================================" << std::endl;
}
