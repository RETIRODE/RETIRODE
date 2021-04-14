using Nancy.TinyIoc;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;
using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public class RangeMeasurementService : IRangeMeasurementService
    {
        //------------- STATIC VARIABLES -------------//
        private static Guid GattServiceId = Guid.Parse("5177db0a-8ce6-11eb-8dcd-0242ac130003");
        private static Guid GattCharacteristicReceiveId = Guid.Parse("00000000000000000000000000000000");
        private static Guid GattCharacteristicSendId = Guid.Parse("5177de8e-8ce6-11eb-8dcd-0242ac130003");
        private static Guid SpecialNotificationDescriptorId = Guid.Parse("00000000000000000000000000000000");
        private static string RetirodeUniqueMacAddressPart = "60:C0:BF";
        private static string UniqueRetirodeName = "Retirode";
        private static int UniqueMacAddressLength = 8;

        //------------- CLASS VARIABLES -------------//
        private readonly IBluetoothService _bluetoothService;
        private IList<IDevice> _availableDevices;
        private IDevice _connectedDevice;
        public IList<BLEDevice> AvailableDevices => _availableDevices.Select(x => new BLEDevice()
        {
            Identifier = x.Id,
            Name = x.Name,
            State = x.State
        }).ToList();

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

        public async Task<bool> ConnectToRSL10(BLEDevice bleDevice)
        {
            bool result = false;
            try
            {
                var device = _availableDevices.FirstOrDefault(foundDevice => foundDevice.Name == bleDevice.Name);
                if (device != null)
                {
                    _connectedDevice = device;
                    await _bluetoothService.ConnectToDeviceAsync(_connectedDevice);
                    result = true;
                }
            }
            catch
            {
                throw new Exception("Cannot connect to device");
            }
            return result;
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
            await _bluetoothService.StartScanning();
        }

        public Task<bool> StopMeasurement()
        {
            throw new NotImplementedException();
        }

        private async void DeviceDiscovered(object sender, IDevice device)
        {
            if (await IsWhiteList(device))
            {
                _availableDevices.Add(device);
            }
        }

        private Task<bool> IsWhiteList(IDevice device)
        {
            if (device.Name == UniqueRetirodeName && IsMacAddressEquals(device.NativeDevice))
            {
                return Task.FromResult(true);
            }
            return Task.FromResult(false);
        }

        private bool IsMacAddressEquals(object device)
        {
            PropertyInfo propertyInfo = device.GetType().GetProperty("Address");
            var macAddress = (string)propertyInfo.GetValue(device, null);

            return macAddress.Substring(0, UniqueMacAddressLength).Equals(RetirodeUniqueMacAddressPart);
        }

    }
}
