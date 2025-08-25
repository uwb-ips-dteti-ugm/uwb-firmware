#include "apps/UWBClient/kernel.h"

void uwc::run(uwc::KernelInst *inst)
{
    xTaskCreatePinnedToCore(
        uwc::kernelTask,
        UWBCLIENT_KERNEL_NAME,
        UWBCLIENT_KERNEL_STACK_SIZE,
        (void *)inst,
        UWBCLIENT_KERNEL_PRIORITY,
        &uwc::kerReg.taskHandle,
        UWBCLIENT_KERNEL_CORE);
}

void uwc::kernelTask(void *pvParameters)
{
    uwc::KernelInst *inst = (uwc::KernelInst *)pvParameters;
    uwc::kernelSetup(inst);

    while (1)
    {
        uwc::kernelLoop(inst);
        vTaskDelay(UWBCLIENT_KERNEL_DELAY_TICKS);
    }
}

void uwc::kernelSetup(uwc::KernelInst *inst)
{
    inst->client = new uwb::DW3000Client();
    inst->client->initBypass();
}

void uwc::kernelLoop(uwc::KernelInst *inst)
{
    uwc::kernelEventRoutine(inst);
    uwc::kernelMainRoutine(inst);
}

void uwc::kernelEventRoutine(uwc::KernelInst *inst)
{
    static uwc::trp::models::Generic buf;

    if (xQueueReceive(uwc::trp::queue, &buf, 0) == pdTRUE)
    {
        switch (buf.type)
        {
        }
    }
}

void uwc::kernelMainRoutine(uwc::KernelInst *inst)
{
    switch (uwc::utl::rGetKernelState())
    {
    case uwc::KERNEL_STATE_CONFIG:
    {
        if (!inst->client->deviceConfig())
        {
            uwc::utl::rSetKernelState(uwc::KERNEL_STATE_CONFIG_FAILED);
        }
        else
        {
            inst->client->networkConfig(inst->deviceAddress, inst->mode);
            uwc::utl::rSetKernelState(uwc::KERNEL_STATE_RUNNING);
        }
    }

    case uwc::KERNEL_STATE_CONFIG_FAILED:
    {
        break;
    }

    case uwc::KERNEL_STATE_RUNNING:
    {
        inst->client->spin();
        break;
    }
    }
}
