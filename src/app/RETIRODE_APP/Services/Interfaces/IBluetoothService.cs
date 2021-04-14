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
        Task StartScanning();

        Task ConnectToDeviceAsync(IDevice btDevice);

        Task<bool> WriteToCharacteristic(ICharacteristic characteristic, RSL10Command stopMeasurement);

        Task ReadFromCharacteristic(ICharacteristic characteristic);

        Action<object, IDevice> DeviceFounded { get; set; }
    }
}
