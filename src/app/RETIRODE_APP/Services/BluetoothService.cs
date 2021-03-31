using Plugin.BLE;
using Plugin.BLE.Abstractions.Contracts;
using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public class BluetoothService : IBluetoothService
    {
        //------------- STATIC VARIABLES -------------//
        private static Guid GattServiceId = Guid.Parse("");
        private static Guid GattCharacteristicReceiveId = Guid.Parse("");
        private static Guid GattCharacteristicSendId = Guid.Parse("");
        private static Guid SpecialNotificationDescriptorId = Guid.Parse("");
        private static string RetirodeUniqueMacAddressPart = "60:C0:BF";
        private static int UniqueMacAddressLength = 8;

        //------------- CLASS VARIABLES -------------//
        private readonly IAdapter _bluetoothAdapter;
        private IDevice _device;
        private List<IDevice> _gattDevices = new List<IDevice>();
        private readonly Queue<IDescriptor> descriptorWriteQueue = new Queue<IDescriptor>();
        private readonly Queue<ICharacteristic> characteristicReadQueue = new Queue<ICharacteristic>();

        public Action<object, IDevice> DeviceFounded { get; set; }

        public BluetoothService()
        {
            _bluetoothAdapter = CrossBluetoothLE.Current.Adapter;
            _bluetoothAdapter.DeviceDiscovered += (obj, device) => DeviceFounded.Invoke(obj,device.Device);
        }

        public bool ConnectToDevice(IDevice btDevice)
        {
            return true;
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

        //private async Task<bool> IsWhiteList(IDevice device)
        //{
        //    var address = GetMacAddressOfDevice(device.NativeDevice);

        //    var deviceUniqueServiceUUID = await device.GetServicesAsync();
        //    foreach(var item in deviceUniqueServiceUUID)
        //    {
        //        if(item.Id == GattServiceId)
        //        {
        //            //TODO
        //        }
        //    }

        //    if (address.Substring(0, UniqueMacAddressLength).Equals(RetirodeUniqueMacAddressPart))
            
        //}

        private string GetMacAddressOfDevice(Object device)
        {
            PropertyInfo propertyInfo = device.GetType().GetProperty("Address");
            return (string)propertyInfo.GetValue(device, null);
        }
    }
}
