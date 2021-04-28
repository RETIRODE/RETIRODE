using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models
{
    public enum RSL10Command : Byte
    {
        StartLidar = 0x01,
        StopLidar = 0x02,
        StartTransfer= 0x03
    }
}
