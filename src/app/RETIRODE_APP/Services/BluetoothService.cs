using Plugin.BLE.Abstractions.Contracts;
using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    class BluetoothService : IBluetoothService
    {
        public bool connectToDevice(IDevice btDevice)
        {
            throw new NotImplementedException();
        }

        public List<IDevice> GetBluetoothDevices()
        {
            throw new NotImplementedException();
        }

        public Task<string> ReadCharacteristic()
        {
            throw new NotImplementedException();
        }

        public Task<bool> WriteCharacteristic()
        {
            throw new NotImplementedException();
        }
    }
}
