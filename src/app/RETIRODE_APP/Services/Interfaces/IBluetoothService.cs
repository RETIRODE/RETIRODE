using Plugin.BLE.Abstractions.Contracts;
using System;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public interface IBluetoothService
    {
        /// <summary>
        /// Starts asynchornous scanning for available devices
        /// nearby
        /// </summary>
        /// <returns></returns>
        Task StartScanning();

        /// <summary>
        /// Connect to device straightforward, also set 
        /// the higher speed of data transport
        /// </summary>
        /// <param name="btDevice" <see cref="IDevice"/>></param>
        /// <returns></returns>
        Task ConnectToDeviceAsync(IDevice btDevice);

        /// <summary>
        /// writing to specific characteristic
        /// </summary>
        /// <param name="characteristic" <see cref="ICharacteristic"/>></param>
        /// <param name="message"></param>
        /// <returns></returns>
        Task<bool> WriteToCharacteristic(ICharacteristic characteristic, byte[] message);

        /// <summary>
        /// event fired when some device nearby is found
        /// </summary>
        Action<object, IDevice> DeviceFound { get; set; }
    }
}
