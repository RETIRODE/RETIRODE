using Plugin.BLE.Abstractions.EventArgs;
using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using static RETIRODE_APP.Models.Enums.ApplicationEnums;

namespace RETIRODE_APP.Services.Interfaces
{
    public interface IRangeMeasurementService : IDisposable
    {
        /// <summary>
        /// This event is fired immadiately when device is found 
        /// and checked by white list
        /// </summary>
        event Action<BLEDevice> DeviceDiscoveredEvent;

        /// <summary>
        /// Event is fired when response from query characteristic
        /// is given
        /// </summary>
        event Action<ResponseItem> QueryResponseEvent;

        /// <summary>
        /// Event is fired when response from measuring is given.
        /// Event involves processed measured data from bluetooth
        /// which are interpretated as array of integers
        /// </summary>
        event Action<List<float>> MeasuredDataResponseEvent;

        /// <summary>
        /// Event is fired as soon as device disconnected
        /// </summary>
        event Action<RangeMeasurementErrorMessages> DeviceDisconnectedEvent;

        /// <summary>
        /// Event is fired when something went wrong with Range Finder (LIDAR)
        /// </summary>
        event Action MeasurementErrorEvent;

        /// <summary>
        /// This method is writing to characteristic to start measuring data
        /// </summary>
        /// <returns></returns>
        Task StartMeasurement();

        /// <summary>
        /// This method is writing to characteristic to stop measuring data
        /// </summary>
        /// <returns></returns>
        Task StopMeasurement();

        /// <summary>
        /// Connect to device by param which is given(tapped) from GUI,
        /// set all available characteristics and services, device has
        /// and start to listening to those characteristic which are in 
        /// read (notify) state
        /// </summary>
        /// <param name="bleDevice"></param>
        /// <returns></returns>
        Task ConnectToRSL10(BLEDevice bleDevice);

        /// <summary>
        /// This method tells bluetooth adapter to start scaning devices nearby
        /// </summary>
        /// <returns></returns>
        Task StartScanning();

        /// <summary>
        /// set lidar to basic settings
        /// </summary>
        /// <returns></returns>
        Task SwReset();

        /// <summary>
        /// set laser voltage value for lidar
        /// </summary>
        /// <param name="laserVoltage"></param>
        /// <returns></returns>
        Task SetLaserVoltage(float laserVoltage);

        /// <summary>
        /// set sipm bias voltage power value for lidar
        /// </summary>
        /// <param name="sipmBiasPowerVoltage"></param>
        /// <returns></returns>
        Task SetSipmBiasPowerVoltage(float sipmBiasPowerVoltage);

        /// <summary>
        /// Start calibrate range finder 
        /// </summary>
        /// <returns></returns>
        Task CalibrateLidar();

        /// <summary>
        /// set pulse count for lidar
        /// </summary>
        /// <returns></returns>
        Task SetPulseCount(int pulseCount);

        /// <summary>
        /// Writes to characteristic to get pulse count
        /// </summary>
        /// <returns></returns>
        Task GetPulseCount();

        /// <summary>
        /// Send request for target or actual value of lidar's
        /// laser voltage
        /// </summary>
        /// <returns></returns>
        Task GetLaserVoltage(Voltage voltage);

        /// <summary>
        /// Send request for target or actual value of lidar's
        /// sipm bias voltage power
        /// </summary>
        /// <returns></returns>
        Task GetSipmBiasPowerVoltage(Voltage voltage);

        /// <summary>
        /// Send request to get status of overload and if
        /// laser and simp bias is turned on or off
        /// </summary>
        /// <returns></returns>
        Task GetVoltagesStatus();

        /// <summary>
        /// Turns on/off laser voltage
        /// </summary>
        /// <param name="switchCase"></param>
        /// <returns></returns>
        Task SwitchLaserVoltage(SwitchState switchCase);


        /// <summary>
        /// Turns on/off sipm bias voltage
        /// </summary>
        /// <param name="switchCase"></param>
        /// <returns></returns>
        Task SwitchSipmBiasVoltage(SwitchState switchCase);

        /// <summary>
        /// Tells bluetooth adapter to stop scanning devices.
        /// </summary>
        /// <returns></returns>
        Task StopScanning();
    }
}
