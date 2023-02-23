#include "platformfs.h"

#include <unistd.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <sstream>

mender::io::ExpectedFile mender::io::Open(const string &p, bool read, bool write) {
	if (!read && !write) {
		return Error(std::error_condition(std::errc::io_error), "Wrong access flags provided");
	}
	int flags = read && write ? O_RDWR | O_CREAT : write ? O_WRONLY | O_CREAT : O_RDONLY;
	int permissions = 0644;
	mender::io::File fd = open(p.c_str(), flags, permissions);

	if (fd < 0) {
		std::stringstream ss;
		ss << "Error opening file: " << errno;
		return Error(std::error_condition(std::errc::io_error), ss.str());
	}
	return fd;
}

Error mender::io::Close(File f) {
	int ret = close(f);
	if (ret < 0) {
		return Error(std::error_condition(std::errc::io_error), "Failed to close file");
	}
	return NoError;
}

ExpectedSize mender::io::GetSize(mender::io::File f) {
	struct stat statbuf;
	if (-1 == fstat(f, &statbuf)) {
		return Error(std::error_condition(std::errc::io_error), "Failed to get fstat");
	}

	size_t size;
	if (S_ISBLK(statbuf.st_mode)) {
		int r = ioctl(f, BLKGETSIZE64, &size);
		if (r < 0) {
			std::stringstream ss;
			ss << "Error getting file size: " << errno;
			return Error(std::error_condition(std::errc::io_error), ss.str());
		}
	} else {
		size = statbuf.st_size;
	}
	return size;
}

ExpectedSize mender::io::Read(File f, Bytes &data)
{
	ssize_t bytesRead = read(f, data.data(), data.size());
	if (bytesRead < 0) {
		return Error(std::error_condition(std::errc::io_error), "Error while reading data");
	}
	return bytesRead;
}

ExpectedSize mender::io::Write(File f, const Bytes &data)
{
	ssize_t bytesWrote = write(f, data.data(), data.size());
	if (bytesWrote <= 0) {
		return Error(std::error_condition(std::errc::io_error), "Error while writing data");
	}
	return bytesWrote;
}

Error mender::io::Flush(File f)
{
	if (0 != fsync(f)) {
		return Error(std::error_condition(std::errc::io_error), "Error while flushing data");
	}
	return NoError;
}

Error mender::io::SeekSet(mender::io::File f, uint64_t pos) {
	if (pos != static_cast<uint64_t>(lseek64(f, pos, SEEK_SET))) {
		return Error(std::error_condition(std::errc::io_error), "Can't set seek on a file");
	}
	return NoError;
}

ExpectedSize mender::io::Tell(mender::io::File f) {
	ssize_t pos = lseek64(f, 0, SEEK_CUR);
	if (pos < 0) {
		std::stringstream ss;
		ss << "Error getting file position: " << errno;
		return Error(std::error_condition(std::errc::io_error), ss.str());
	}
	return pos;
}

mender::io::File mender::io::GetInputStream() {
	return STDIN_FILENO;
}

ExpectedBool mender::io::IsSpecialBlockDevice(File f) {
	struct stat statbuf;
	if (-1 == fstat(f, &statbuf)) {
		return Error(std::error_condition(std::errc::io_error), "Failed to get fstat");
	} else if (S_ISBLK(statbuf.st_mode)) {
		return true;
	}
	return false;
}

ExpectedSize mender::io::WriteFile(const string &path, const Bytes &data) {
	auto f = mender::io::Open(std::move(path), false, true);
	if (f) {
		auto fd = f.value();
		ssize_t bytesWrote = write(fd, data.data(), data.size());
		Close(fd);
		if (bytesWrote <= 0) {
			return Error(std::error_condition(std::errc::io_error), "Error while writing data");
		} else {
			return bytesWrote;
		}
	}
	return f.error();
}

ExpectedString mender::io::MakeTempDir(const string &templateName) {
	std::string name = templateName + "XXXXXX";
	char *dir_name = mkdtemp(const_cast<char *>(name.c_str()));
	if (dir_name == nullptr) {
		return Error(std::error_condition(std::errc::io_error), "Creating temp dir");
	}
	return name;
}
