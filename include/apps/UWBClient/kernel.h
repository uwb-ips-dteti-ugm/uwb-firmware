#ifndef __UWBCLIENT_KERNEL_H__
#define __UWBCLIENT_KERNEL_H__

namespace uwc
{
    struct KernelInst
    {
    };

    void runKernel(KernelInst *inst);
    void kernelTask(void *pvParameters);
    void kernelSetup();
    void kernelLoop();
    void eventRoutine();
    void keepRoutine();

    namespace fnc
    {
    }
}

#endif