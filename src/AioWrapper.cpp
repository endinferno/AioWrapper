#include "AioWrapper.hpp"
#include <libaio.h>

struct WrapperCFunc
{
public:
    static void WrapperFunc(io_context_t ioCtx, struct iocb* ioConfig,
                            std::int64_t res, std::int64_t res2)
    {
        replacedFunc(ioCtx, ioConfig, res, res2);
    }
    inline static AioWrapper::ReadCallback replacedFunc;
};

AioWrapper::AioWrapper(ReadCallback readCallback)
    : ioConfigs_(AIO_MAXIO_)
    , events_(AIO_MAXIO_)
{
    InitIoContext();
    InitIoConfig();
    WrapperCFunc::replacedFunc = std::move(readCallback);
}

AioWrapper::~AioWrapper()
{
    DeinitIoConfig();
    DeinitIoContext();
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
        if (WrapperCFunc::replacedFunc != nullptr) {
            ::io_set_callback(ioConfigs_[i], &WrapperCFunc::WrapperFunc);
        }
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
    int totalCompleEvt = 0;
    while (true) {
        int completeEvt =
            ::io_getevents(ioCtx_, 1, AIO_MAXIO_, events_.data(), nullptr);
        totalCompleEvt += completeEvt;
        for (int i = 0; i < completeEvt; i++) {
            auto callback = reinterpret_cast<io_callback_t>(events_[i].data);
            if (callback == nullptr) {
                continue;
            }

            callback(ioCtx_,
                     events_[i].obj,
                     static_cast<std::int64_t>(events_[i].res),
                     static_cast<std::int64_t>(events_[i].res2));
        }
        if (totalCompleEvt == submitNum) {
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

void AioWrapper::DeinitIoContext()
{
    ::io_destroy(ioCtx_);
}

void AioWrapper::DeinitIoConfig()
{
    for (auto& ioConfig : ioConfigs_) {
        delete ioConfig;
    }
}
