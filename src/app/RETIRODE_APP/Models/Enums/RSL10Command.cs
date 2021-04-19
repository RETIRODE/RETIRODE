using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models
{
    public enum RSL10Command : Byte
    {
        StartMeasurement = 0x00,
        StopMeasurement = 0x01
    }
}
