using Plugin.BLE.Abstractions.Contracts;
using Plugin.BLE.Abstractions.EventArgs;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services.Interfaces
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
        /// <returns>
        /// True - if write to characteristic was correct
        /// False - if write to characteristic was wrong
        /// </returns>
        Task<bool> WriteToCharacteristic(ICharacteristic characteristic, byte[] message);
        
        /// <summary>
        /// Tells BT adapter to stop scanning devices
        /// </summary>
        /// <returns></returns>
        Task StopScanning();

        /// <summary>
        /// event fired when some device nearby is found
        /// </summary>
        Action<object, IDevice> DeviceFound { get; set; }

        /// <summary>
        /// Event is fired when device is disconnected
        /// </summary>
        Action<object,DeviceEventArgs> DeviceDisconnected { get; set; }

        /// <summary>
        /// State of bluetooth adapter
        /// True - if BT adapter is scanning
        /// False - if BT adapter is not scanning
        /// </summary>
        bool IsScanning { get; }

        /// <summary>
        /// list of connected devices
        /// </summary>
        IReadOnlyList<IDevice> ConnectedDevices { get; }

        /// <summary>
        /// Investigate if device is still connected to mobile device
        /// </summary>
        /// <returns>
        /// True - if device is connected
        /// False - if device is not connected
        /// </returns>
        Task<bool> IsDeviceConnected();
    }
}
