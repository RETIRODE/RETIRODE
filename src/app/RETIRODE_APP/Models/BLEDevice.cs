using Plugin.BLE.Abstractions;
using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models
{
    public class BLEDevice
    {
        public Guid Identifier { get; set; }

        public string Name { get; set; }
        
        public DeviceState State { get; set; }

    }

}
