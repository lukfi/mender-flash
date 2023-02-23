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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include "fileio.hpp"
#include "optimized_block_device_writer.hpp"

class OptimizedBlockWriterTests : public testing::Test {
protected:
	void SetUp() override {
	}

	void TearDown() override {
		std::filesystem::remove_all(mTempDir);
	}
	std::string mTempDir;
};

TEST_F(OptimizedBlockWriterTests, TestBlockDeviceWrite) {
	// prepare a temp dir
	auto dir = mender::io::MakeTempDir("mender-block-device-");
	ASSERT_TRUE(dir) << dir.error().message;
	mTempDir = dir.value();
	auto path = mTempDir + "/foo";

	// Write some junk to the file
	auto s = mender::io::WriteFile(path, {'a', 'b', 'x', 'd', 'r', 'z', '1', '2', '3', '4'});
	ASSERT_TRUE(s) << s.error().message;

	auto f = mender::io::Open(path, true, true);
	ASSERT_TRUE(f) << f.error().message;
	mender::io::File fd = f.value();


	// set a limit to 10bytes
	mender::OptimizedBlockDeviceWriter writer(fd, 10);

	mender::io::Bytes payloadBuf {'f', 'o', 'o', 'b', 'a', 'r'};
	auto expectedBytesWritten = payloadBuf.size();

	auto res = writer.Write(payloadBuf);
	ASSERT_TRUE(res) << res.error().message;
	ASSERT_EQ(res.value(), expectedBytesWritten);

	auto err = mender::io::Close(fd);
	ASSERT_EQ(err, NoError);
}

TEST_F(OptimizedBlockWriterTests, TestBlockDeviceSize) {
	// prepare a temp dir
	auto dir = mender::io::MakeTempDir("mender-block-device-");
	ASSERT_TRUE(dir) << dir.error().message;
	mTempDir = dir.value();
	auto path = mTempDir + "/foo";

	auto f = mender::io::Open(path, true, true);
	ASSERT_TRUE(f) << f.error().message;
	mender::io::File fd = f.value();

	// set a limit to 10bytes
	mender::OptimizedBlockDeviceWriter writer(fd, 10);

	// create a 12 byte buffer
	mender::io::Bytes payloadBuf {'f', 'o', 'o', 'b', 'a', 'r', 'f', 'o', 'o', 'b', 'a', 'r'};
	// auto payloadSize = payloadBuf.size();

	auto res = writer.Write(payloadBuf);
	ASSERT_FALSE(res) << "Data written beyound the device limit";

	auto err = mender::io::Close(fd);
	ASSERT_EQ(err, NoError);
}

TEST_F(OptimizedBlockWriterTests, TestBlockFrameWriter) {
	struct test {
		int frameSize;
		mender::io::Bytes input;
		mender::io::Bytes expected;
		int expectedBytesWritten;
		int expectedBytesCached;
	} tests[] = {
		{2, {'f', 'o'}, {'f', 'o'}, 2, 0},
		{6, {'f', 'o'}, {}, 3, 3},
		{4, {'f', 'o', 'o', 'b', 'a', 'r'}, {'f', 'o', 'o', 'b'}, 6, 2}};

	for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
		// TODO
	}
}

int main(int argc, char *argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
