#include "apps/UWBServer/kernel.h"

void uws::run(uws::KernelInst *inst)
{
    xTaskCreatePinnedToCore(
        uws::kernelTask,
        UWBSERVER_KERNEL_NAME,
        UWBSERVER_KERNEL_STACK_SIZE,
        (void *)inst,
        UWBSERVER_KERNEL_PRIORITY,
        &uws::kerReg.taskHandle,
        UWBSERVER_KERNEL_CORE);
}

void uws::kernelTask(void *pvParameters)
{
    uws::KernelInst *inst = (uws::KernelInst *)pvParameters;
    uws::kernelSetup(inst);

    while (1)
    {
        uws::kernelLoop(inst);
        vTaskDelay(UWBSERVER_KERNEL_DELAY_TICKS);
    }
}

void uws::kernelSetup(KernelInst *inst)
{
    inst->server = new uwb::DW3000Server(inst->clientMax);
}

void uws::kernelLoop(KernelInst *inst)
{
    uws::kernelEventRoutine(inst);
    uws::kernelMainRoutine(inst);
}

void uws::kernelEventRoutine(KernelInst *inst)
{
    static uws::trp::models::Generic buf;

    if (xQueueReceive(uws::trp::queue, &buf, 0) == pdTRUE)
    {
        switch (buf.type)
        {
        }
    }
}

void uws::kernelMainRoutine(KernelInst *inst)
{
    switch (uws::utl::rGetKernelState())
    {
    case uws::KERNEL_STATE_CONFIG:
    {
        if (!inst->server->deviceConfig())
        {
            uws::utl::rSetKernelState(uws::KERNEL_STATE_CONFIG_FAILED);
        }
        else
        {
            inst->server->networkConfig(inst->networkAddress, inst->deviceAddress);
            uws::utl::rSetKernelState(uws::KERNEL_STATE_RUNNING);
        }
    }

    case uws::KERNEL_STATE_CONFIG_FAILED:
    {
        break;
    }

    case uws::KERNEL_STATE_RUNNING:
    {
        inst->server->spin();
        uws::utl::rUpdateNetworkInfo(inst->server);
        break;
    }
    }
}
