using RETIRODE_APP.Models;
using RETIRODE_APP.Models.Enums;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public interface IRangeMeasurementService : IDisposable
    {
        /// <summary>
        /// 
        /// </summary>
        event Action<BLEDevice> DeviceDiscoveredEvent;

        /// <summary>
        /// 
        /// </summary>
        event Action<ResponseItem> QueryResponseEvent;

        /// <summary>
        /// 
        /// </summary>
        event Action QueryDataReceivedEvent;

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
        /// 
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
        /// 
        /// </summary>
        /// <returns></returns>
        Task SwReset();

        /// <summary>
        /// 
        /// </summary>
        /// <param name="laserVoltage"></param>
        /// <returns></returns>
        Task SetLaserVoltage(int laserVoltage);

        /// <summary>
        /// 
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
        /// 
        /// </summary>
        /// <returns></returns>
        Task SetPulseCount(int pulseCount);

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        Task GetLaserVoltage(Voltage voltage);

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        Task GetSipmBiasPowerVoltage(Voltage voltage);

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        Task GetCalibration(Calibrate calibrate);
    }
}
