using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models.Enums
{
    public enum Registers : Byte
    {
        SWReset = 0x00,
        LaserVoltage = 0x01,
        SipmBiasPowerVoltage = 0x02,
        Calibrate = 0x03,
        PulseCount = 0x04
    }
}
