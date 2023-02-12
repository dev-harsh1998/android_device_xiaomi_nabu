package custom.hardware.hwcontrol;

import custom.hardware.hwcontrol.HwType;

interface IHwControl {
    int getHwState(in HwType hwType);
    oneway void setHwState(in HwType hwType, int state);
}