using Plugin.BLE;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;
using Plugin.BLE.Abstractions.EventArgs;
using RETIRODE_APP.Services.Interfaces;
using System;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    /// <inheritdoc cref="IBluetoothService"/>
    public class BluetoothService : IBluetoothService
    {
        private readonly IAdapter _bluetoothAdapter;
        private IDevice _device;

        /// <inheritdoc cref="IBluetoothService"/>
        public Action<object, IDevice> DeviceFound { get; set; }

        /// <inheritdoc cref="IBluetoothService"/>
        public Action<object,DeviceEventArgs> DeviceDisconnected { get; set; }

        public BluetoothService()
        {
            _bluetoothAdapter = CrossBluetoothLE.Current.Adapter;
            _bluetoothAdapter.DeviceDiscovered += (obj, device) => DeviceFound.Invoke(obj, device.Device);
            _bluetoothAdapter.DeviceDisconnected += (obj, e) => DeviceDisconnected.Invoke(obj,e);
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
        public Task StartScanning()
        {
            return _bluetoothAdapter.StartScanningForDevicesAsync();            
        }

        /// <inheritdoc cref="IBluetoothService"/>
        public bool IsScanning => _bluetoothAdapter.IsScanning;

        /// <inheritdoc cref="IBluetoothService"/>
        public Task StopScanning()
        {
            return _bluetoothAdapter.StopScanningForDevicesAsync();
        }

        /// <inheritdoc cref="IBluetoothService"/>
        public Task<bool> WriteToCharacteristic(ICharacteristic characteristic, byte[] message)
        {
            return characteristic.WriteAsync(message);
        }

        /// <inheritdoc cref="IBluetoothService"/>
        public Task<bool> IsDeviceConnected()
        {
            var connectedDevices = _bluetoothAdapter.ConnectedDevices;

            foreach (var device in connectedDevices)
            {
                if(device == _device)
                {
                    return Task.FromResult(true);
                }
            }
            return Task.FromResult(false);
        }

    }
}
