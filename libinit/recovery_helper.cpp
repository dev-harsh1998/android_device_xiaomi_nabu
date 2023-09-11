#include "recovery_helper.h"

void recovery_helper()
{
    // We should directly return if the device is not in recovery mode.
    if (!android::init::IsRecoveryMode())
    {
        LOG(ERROR) << "Not in recovery mode, exiting.";
        return;
    }

    LOG(INFO) << "Starting recovery helper";
}
