#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <vector>

#include <libaio.h>

struct AioInfo
{
    int fileFd;
    void* readBuf;
    std::size_t readSize;
    std::int64_t offset;
};

class AioWrapper
{
public:
    using ReadCallback = std::function<void(io_context_t, struct iocb*,
                                            std::int64_t, std::int64_t)>;
    explicit AioWrapper(ReadCallback readCallback);
    void Read(std::vector<AioInfo>& aioInfos, bool sync = false);
    ~AioWrapper();

private:
    void InitIoContext();
    void InitIoConfig();
    void Submit(std::int64_t submitNum, bool sync);
    void WaitReqComplete(std::int64_t submitNum);

    const int AIO_MAXIO_ = 256;
    io_context_t ioCtx_;
    std::vector<struct iocb*> ioConfigs_;
    std::vector<struct io_event> events_;
    ReadCallback readCallback_;
};
