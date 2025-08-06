#include "apps/UWBClient/utils.h"

uwc::KernelState uwc::utl::rGetKernelState()
{
    uwc::KernelState state = uwc::kerReg.kernelState;
    return state;
}

void uwc::utl::rSetKernelState(uwc::KernelState state)
{
    uwc::kerReg.kernelState = state;
}