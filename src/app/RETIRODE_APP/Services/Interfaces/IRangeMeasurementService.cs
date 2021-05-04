using RETIRODE_APP.Models;
using RETIRODE_APP.Models.Enums;
using System;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
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
        event Action<int[]> MeasuredDataResponseEvent;

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        Task StartMeasurement();

        /// <summary>
        /// 
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
        /// 
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
        Task SetLaserVoltage(int laserVoltage);

        /// <summary>
        /// set sipm bias voltage power value for lidar
        /// </summary>
        /// <param name="sipmBiasPowerVoltage"></param>
        /// <returns></returns>
        Task SetSipmBiasPowerVoltage(int sipmBiasPowerVoltage);

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        Task CalibrateLidar();

        /// <summary>
        /// set pulse count for lidar
        /// </summary>
        /// <returns></returns>
        Task SetPulseCount(int pulseCount);

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
        /// Send request for one of three calibration values
        /// of lidar
        /// </summary>
        /// <returns></returns>
        Task GetCalibration(Calibrate calibrate);
        Task StopScanning();
    }
}
