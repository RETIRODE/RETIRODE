using Nancy.TinyIoc;
using Plugin.BLE.Abstractions.Contracts;
using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public class RangeMeasurementService : IRangeMeasurementService
    {
        private readonly IBluetoothService _bluetoothService;
        private IList<IDevice> _availableDevices;
        public IList<BLEDevice> AvailableDevices { get; private set; }

        public RangeMeasurementService()
        {
            _availableDevices = new List<IDevice>();
            _bluetoothService = TinyIoCContainer.Current.Resolve<IBluetoothService>();
            _bluetoothService.DeviceFounded = DeviceDiscovered;
        }

        public Task<bool> CalibrateLIDAR()
        {
            throw new NotImplementedException();
        }

        public Task<bool> ConnectToRSL10(BLEDevice bleDevice)
        {
            throw new NotImplementedException();
        }

        public Task<bool> Disconnect(BLEDevice device)
        {
            throw new NotImplementedException();
        }

        public Task<bool> StartMeasurement()
        {
            throw new NotImplementedException();
        }

        public async Task StartScanning()
        {
            _availableDevices.Clear();
            AvailableDevices.Clear();
            await _bluetoothService.StartScanning();
        }

        public Task<bool> StopMeasurement()
        {
            throw new NotImplementedException();
        }

        private void DeviceDiscovered(object sender, IDevice device)
        {
            AvailableDevices.Add(new BLEDevice
            {
                Name = device.Name,
                Identifier = device.Id
            });

            _availableDevices.Add(device);
        }
    }
}
