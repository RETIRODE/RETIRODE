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
    /// <inheritdoc cref="IBluetoothService"/>
    public class BluetoothService : IBluetoothService
    {
        private readonly IAdapter _bluetoothAdapter;
        private IDevice _device;

        /// <inheritdoc cref="IBluetoothService"/>
        public Action<object, IDevice> DeviceFound { get; set; }

        public BluetoothService()
        {
            _bluetoothAdapter = CrossBluetoothLE.Current.Adapter;
            _bluetoothAdapter.DeviceDiscovered += (obj, device) => DeviceFound.Invoke(obj,device.Device);
        }

        /// <inheritdoc cref="IBluetoothService"/>
        public async Task ConnectToDeviceAsync(IDevice btDevice)
        {
                _device = btDevice;
                var connectParameters = new ConnectParameters(false, true);
                await _bluetoothAdapter.ConnectToDeviceAsync(btDevice, connectParameters);

                //Connection interval to increase speed
                //Does not work on iOS devices. Supported only on Android API > 21
                _device.UpdateConnectionInterval(ConnectionInterval.High);
        }

        /// <inheritdoc cref="IBluetoothService"/>
        public async Task StartScanning()
        {
            await _bluetoothAdapter.StartScanningForDevicesAsync();
        }

        /// <inheritdoc cref="IBluetoothService"/>
        public async Task<bool> WriteToCharacteristic(ICharacteristic characteristic, byte[] message)
        {
             return await characteristic.WriteAsync(message);
        }
    }
}
