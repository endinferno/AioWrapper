#include "AioWrapper.hpp"
#include <libaio.h>

AioWrapper::AioWrapper(ReadCallback readCallback)
    : ioConfigs_(AIO_MAXIO_)
    , events_(AIO_MAXIO_)
    , readCallback_(std::move(readCallback))
{
    InitIoContext();
    InitIoConfig();
}

AioWrapper::~AioWrapper()
{
    ::io_destroy(ioCtx_);
}

void AioWrapper::Read(std::vector<AioInfo>& aioInfos, bool sync)
{
    for (size_t i = 0; i < aioInfos.size(); i++) {
        auto& info = aioInfos[i];
        ::io_prep_pread(ioConfigs_[i],
                        info.fileFd,
                        info.readBuf,
                        info.readSize,
                        info.offset);
    }
    Submit(static_cast<std::int64_t>(aioInfos.size()), sync);
}

void AioWrapper::Submit(std::int64_t submitNum, bool sync)
{
    int ret = ::io_submit(ioCtx_, submitNum, ioConfigs_.data());
    if (ret < 0) {
        printf("Fail to submit aio requests\n");
    }
    if (sync) {
        WaitReqComplete(submitNum);
    }
}

void AioWrapper::WaitReqComplete(std::int64_t submitNum)
{
    int completeEvt = 0;
    while (true) {
        completeEvt +=
            ::io_getevents(ioCtx_, 1, AIO_MAXIO_, events_.data(), nullptr);
        if (completeEvt == submitNum) {
            break;
        }
    }
}

void AioWrapper::InitIoContext()
{
    std::memset(&ioCtx_, 0, sizeof(io_context_t));
    int ret = ::io_setup(AIO_MAXIO_, &ioCtx_);
    if (ret < 0) {
        printf("Fail to set aio context\n");
    }
}

void AioWrapper::InitIoConfig()
{
    for (auto& ioConfig : ioConfigs_) {
        ioConfig = new iocb();
    }
}
