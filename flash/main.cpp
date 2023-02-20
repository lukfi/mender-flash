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

#include <iostream>

#include "fileio.hpp"
#include "optimized_block_device_writer.hpp"

int main(int argc, char *argv[]) {
	mender::io::File srcFile;
	mender::io::File dstFile;

	for (int i = 0; i < argc; ++i) {
		printf("argv[%d]: %s\n", i, argv[i]);
	}
	if (argc < 2 || argc > 3) {
		std::cerr << "Wrong input arguments" << std::endl;
		std::cerr << "usage: mender-flash [srcPath] dstPath" << std::endl;
		return 1;
	}
	std::shared_ptr<mender::io::FileReader> reader;
	char *dstPath;
	if (argc == 2) {
		srcFile = mender::io::GetInputStream();
		reader = std::make_shared<mender::io::FileReader>(srcFile);
		dstPath = argv[1];
	} else {
		auto src = mender::io::Open(argv[1]);
		if (!src) {
			std::cerr << "Failed to open src: " << argv[1] << " (" << src.error().message << ")";
			return 1;
		}
		srcFile = src.value();
		reader = std::make_shared<mender::io::FileReader>(srcFile);
		dstPath = argv[2];
	}

	auto dst = mender::io::Open(dstPath, true, true);
	if (!dst) {
		std::cerr << "Failed to open dst: " << dstPath << " (" << dst.error().message << ")";
		return 1;
	}
	dstFile = dst.value();
	mender::OptimizedBlockDeviceWriter optWriter(dstFile, 0, true);

	std::vector<uint8_t> v;
	v.resize(1024*1024);
	while (reader->Read(v)) {
		optWriter.Write(v);
	}

	if (srcFile != mender::io::GetInputStream()) {
		mender::io::Close(srcFile);
	}
	mender::io::Close(dstFile);

	optWriter.PrintStatistics();
	return 0;
}
