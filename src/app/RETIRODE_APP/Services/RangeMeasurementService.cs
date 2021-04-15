using Nancy.TinyIoc;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;
using Plugin.BLE.Abstractions.EventArgs;
using RETIRODE_APP.Models;
using RETIRODE_APP.Models.Enums;
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
        private static Guid GattFirstServiceId = Guid.Parse("5177db0a-8ce6-11eb-8dcd-0242ac130003");
        private static Guid GattFirstCharacteristicReceiveId = Guid.Parse("00000000000000000000000000000000");
        private static Guid GattFirstCharacteristicWriteId = Guid.Parse("5177de8e-8ce6-11eb-8dcd-0242ac130003");

        private static Guid GattSecondCharacteristicReceiveId = Guid.Parse("00000000000000000000000000000000");
        private static Guid GattSecondCharacteristicWriteId = Guid.Parse("00000000000000000000000000000000");
        private static Guid GattSecondServiceId = Guid.Parse("00000000000000000000000000000000");

        private static readonly string RetirodeUniqueMacAddressPart = "60:C0:BF";
        private static readonly string UniqueRetirodeName = "Retirode";
        private static readonly int UniqueMacAddressLength = 8;

        //------------- CLASS VARIABLES -------------//
        private readonly IBluetoothService _bluetoothService;
        private IList<IDevice> _availableDevices;
        private IDevice _connectedDevice;
        private IList<BLEDevice> AvailableDevices => _availableDevices.Select(x => new BLEDevice()
        {
            Identifier = x.Id,
            Name = x.Name,
            State = x.State
        }).ToList();

        IList<BLEDevice> IRangeMeasurementService.AvailableDevices => throw new NotImplementedException();

        private ICharacteristic _firstCharacteristicDataReceive;
        private ICharacteristic _firstCharacteristicDataWrite;
        private IService _firstService;
        private ICharacteristic _secondCharacteristicDataReceive;
        private ICharacteristic _secondCharacteristicDataWrite;
        private IService _secondService;


        public RangeMeasurementService()
        {
            _availableDevices = new List<IDevice>();
            _bluetoothService = TinyIoCContainer.Current.Resolve<IBluetoothService>();
            _bluetoothService.DeviceFounded = DeviceDiscovered;
        }

        event Action<BLEDevice> DeviceDiscoveredEvent;

        event Action<string> BluetoothResponseEvent;

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
                    Init();
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

        public async Task StartMeasurement()
        {
            BeforeWriteToCharacteristic(_firstCharacteristicDataWrite,_firstService);
            await WriteToCharacteristic(_firstCharacteristicDataWrite, (byte)RSL10Command.StartMeasurement);
        }

        private void BeforeWriteToCharacteristic(ICharacteristic characteristic, IService service)
        {
            if (characteristic is null)
            {
                throw new Exception("Error with send command to characteristic");
            }

            if (service is null)
            {
                throw new Exception("Error with send command to characteristic");
            }
        }
        public async Task StartScanning()
        {
            _availableDevices.Clear();
            await _bluetoothService.StartScanning();
        }

        public async Task StopMeasurement()
        {
            BeforeWriteToCharacteristic(_firstCharacteristicDataWrite, _firstService);
            await WriteToCharacteristic(_firstCharacteristicDataWrite, (byte)RSL10Command.StopMeasurement);
        }

        private async Task WriteToCharacteristic(ICharacteristic characteristic, byte command)
        {
            try
            {
                if(!await _bluetoothService.WriteToCharacteristic(characteristic, command))
                {
                    throw new Exception("Error with send command to characteristic");
                }
            }
            catch(Exception e)
            {
                throw new Exception(e.Message);
            }
        }

        private async void DeviceDiscovered(object sender, IDevice device)
        {
            if (await IsWhiteList(device))
            {
                _availableDevices.Add(device);
                DeviceDiscoveredEvent.Invoke(new BLEDevice()
                {
                    Name = device.Name,
                    Identifier = device.Id,
                    State = device.State
                });
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

        public void Dispose()
        {
            _connectedDevice.Dispose();
        }

        private async void Init()
        {
            _firstService = await _connectedDevice.GetServiceAsync(GattFirstServiceId);
            _firstCharacteristicDataReceive = await _firstService.GetCharacteristicAsync(GattFirstCharacteristicReceiveId);
            _firstCharacteristicDataWrite = await _firstService.GetCharacteristicAsync(GattFirstCharacteristicWriteId);

            _secondService = await _connectedDevice.GetServiceAsync(GattSecondServiceId);
            _secondCharacteristicDataReceive = await _secondService.GetCharacteristicAsync(GattSecondCharacteristicReceiveId);
            _secondCharacteristicDataWrite = await _secondService.GetCharacteristicAsync(GattSecondCharacteristicWriteId);
        }

        public void ReadFromDevice()
        {
            _firstCharacteristicDataReceive.ValueUpdated += QueryResponseHandler;
        }

        private async void QueryResponseHandler(object sender, CharacteristicUpdatedEventArgs e)
        {
            var data = await e.Characteristic.ReadAsync();

            BluetoothResponseEvent.Invoke(Encoding.UTF8.GetString(data));

            //with final data
            //var responseItem = getQueryResponseItem(data);
            //BluetoothResponseEvent.Invoke(responseItem);
        }

    }
}
