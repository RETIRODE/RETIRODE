using Plugin.BLE.Abstractions.Contracts;
using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Services
{
    
    public class BluetoothHandler
    {
        //------------- STATIC VARIABLES -------------//
        private static Guid GattServiceId = Guid.Parse("");
        private static Guid GattCharacteristicReceiveId = Guid.Parse("");
        private static Guid GattCharacteristicSendId = Guid.Parse("");
        private static Guid SpecialNotificationDescriptorId = Guid.Parse("");

        //------------- CLASS VARIABLES -------------//
        private readonly IAdapter _bluetoothAdapter;
        private IDevice _device;
        private List<IDevice> _gattDevices = new List<IDevice>();
        private readonly Queue<IDescriptor> descriptorWriteQueue = new Queue<IDescriptor>();
        private readonly Queue<ICharacteristic> characteristicReadQueue = new Queue<ICharacteristic>();
        

        public BluetoothHandler(IDevice device)
        {

        }

        public async void SendCharacteristic()
        {
            //var externalSensorService = await _device.GetServiceAsync(GattServiceId);
            //var characteristic = await externalSensorService.GetCharacteristicsAsync();

        }
    }
}
