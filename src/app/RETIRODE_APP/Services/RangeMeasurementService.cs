using Nancy.TinyIoc;
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
        private static Guid GattCharacteristicReceiveId = Guid.Parse("");
        private static Guid GattCharacteristicSendId = Guid.Parse("");
        private static Guid SpecialNotificationDescriptorId = Guid.Parse("");
        private static string RetirodeUniqueMacAddressPart = "60:C0:BF";
        private static int UniqueMacAddressLength = 8;

        //------------- CLASS VARIABLES -------------//
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

        public async Task<bool> ConnectToRSL10(BLEDevice bleDevice)
        {
            return await Task.FromResult(true);
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

        private async void DeviceDiscovered(object sender, IDevice device)
        {
            if (await IsWhiteList(device))
            {
                AvailableDevices.Add(new BLEDevice
                {
                    Name = device.Name,
                    Identifier = device.Id
                });

                _availableDevices.Add(device);
            }
        }

        private async Task<bool> IsWhiteList(IDevice device)
        {
            var deviceServices = await device.GetServicesAsync();

            foreach (var service in deviceServices)
            {
                if (service.Id == GattServiceId && IsMacAddressEquals(device))
                {
                    return true;
                }
            }
            return false;
        }

        private bool IsMacAddressEquals(object device)
        {
            PropertyInfo propertyInfo = device.GetType().GetProperty("Address");
            var macAddress = (string)propertyInfo.GetValue(device, null);

            return macAddress.Substring(0, UniqueMacAddressLength).Equals(RetirodeUniqueMacAddressPart);

        }
    }
}
