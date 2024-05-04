#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>

#include "AioWrapper.hpp"

void ReadCallback([[maybe_unused]] io_context_t ioCtx, struct iocb* ioConfig,
                  std::int64_t res, std::int64_t res2)
{
    auto iosize = static_cast<std::int64_t>(ioConfig->u.c.nbytes);

    if (res2 != 0) {
        printf("Fail to aio read\n");
    }
    if (res != iosize) {
        printf("Read missing bytes expect %lu got %ld\n", iosize, res);
        exit(1);
    }
    printf("Read Size %ld\n", res);
}

int OpenFile(const std::string& fileName)
{
    int fileFd = ::open(fileName.c_str(), O_RDWR | O_DIRECT);
    if (fileFd < 0) {
        printf("Fail to open file\n");
        exit(1);
    }
    return fileFd;
}

void* AllocAlign(size_t allocSize)
{
    void* readBuf = nullptr;
    int ret = posix_memalign(&readBuf, getpagesize(), allocSize);
    if (ret < 0) {
        printf("Fail to posix_memalign\n");
        exit(1);
    }
    return readBuf;
}


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    constexpr int READ_SIZE = 1024;
    AioWrapper aioWrapper(ReadCallback);
    int fileFd = OpenFile("test");
    void* readBuf = AllocAlign(READ_SIZE);

    std::vector<AioInfo> aioInfos;
    aioInfos.push_back({ .fileFd = fileFd,
                         .readBuf = readBuf,
                         .readSize = READ_SIZE,
                         .offset = 0 });

    aioWrapper.Read(aioInfos, true);
    return 0;
}
