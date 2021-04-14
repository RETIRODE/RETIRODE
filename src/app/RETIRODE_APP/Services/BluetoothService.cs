using Plugin.BLE;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;
using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace RETIRODE_APP.Services
{
    public class BluetoothService : IBluetoothService
    {
        //------------- CLASS VARIABLES -------------//
        private readonly IAdapter _bluetoothAdapter;
        private IDevice _device;

        public Action<object, IDevice> DeviceFounded { get; set; }

        public BluetoothService()
        {
            _bluetoothAdapter = CrossBluetoothLE.Current.Adapter;
            _bluetoothAdapter.DeviceDiscovered += (obj, device) => DeviceFounded.Invoke(obj,device.Device);
        }

        public async Task ConnectToDeviceAsync(IDevice btDevice)
        {
                _device = btDevice;
                var connectParameters = new ConnectParameters(false, true);
                await _bluetoothAdapter.ConnectToDeviceAsync(btDevice, connectParameters);

                //Connection interval to increase speed
                //Does not work on iOS devices. Supported only on Android API > 21
                _device.UpdateConnectionInterval(ConnectionInterval.High);
        }

        public async Task StartScanning()
        {
            await _bluetoothAdapter.StartScanningForDevicesAsync();
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
