using Plugin.BLE.Abstractions.Contracts;
using System;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public interface IBluetoothService
    {
        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        Task StartScanning();

        /// <summary>
        /// 
        /// </summary>
        /// <param name="btDevice" <see cref="IDevice"/>></param>
        /// <returns></returns>
        Task ConnectToDeviceAsync(IDevice btDevice);

        /// <summary>
        /// 
        /// </summary>
        /// <param name="characteristic" <see cref="ICharacteristic"/>></param>
        /// <param name="message"></param>
        /// <returns></returns>
        Task<bool> WriteToCharacteristic(ICharacteristic characteristic, byte[] message);

        /// <summary>
        /// 
        /// </summary>
        Action<object, IDevice> DeviceFound { get; set; }
    }
}
