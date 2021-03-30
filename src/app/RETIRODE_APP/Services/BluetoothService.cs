using Plugin.BLE.Abstractions.Contracts;
using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public class BluetoothService : IBluetoothService
    {
        public bool ConnectToDevice(IDevice btDevice)
        {
            return true;
        }

        public List<IDevice> GetBluetoothDevices()
        {
            return new List<IDevice>();
        }

        public Task<string> ReadFromCharacteristic()
        {
            return Task.FromResult("Read Characteristic not implemented");
        }

        public Task<bool> WriteToCharacteristic()
        {
            return Task.FromResult(true);
        }
    }
}
