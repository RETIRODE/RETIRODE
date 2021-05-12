using System;

namespace RETIRODE_APP.Models.Enums
{
    public enum Voltage : Byte
    {
        Target = 0x01,
        Actual = 0x02,
        Switch = 0x03
    }
}
