using Plugin.BLE.Abstractions;
using System;

namespace RETIRODE_APP.Models
{
    public class BLEDevice
    {
        public Guid Identifier { get; set; }

        public string Name { get; set; }

        public DeviceState State { get; set; }

    }

}
