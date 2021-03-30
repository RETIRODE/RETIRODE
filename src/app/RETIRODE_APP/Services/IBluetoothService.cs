using Plugin.BLE.Abstractions.Contracts;
using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public interface IBluetoothService
    {
        List<IDevice> GetBluetoothDevices();

        bool ConnectToDevice(IDevice btDevice);

        Task<bool> WriteCharacteristic();

        Task<string> ReadCharacteristic();
    }
}
