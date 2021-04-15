using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public interface IRangeMeasurementService : IDisposable
    {
        event Action<BLEDevice> DeviceDiscoveredEvent;

        event Action<string> BluetoothResponseEvent;
        Task StartMeasurement();

        Task StopMeasurement();

        Task<bool> ConnectToRSL10(BLEDevice bleDevice);

        Task StartScanning();

        void ReadFromDevice();

        Task<bool> CalibrateLIDAR();

        Task<bool> Disconnect(BLEDevice bleDevice);

        Task SwReset();

        Task SetLaserVoltage();

        Task SetSipmBiasPowerVoltage();

        Task Calibrate();
        IList<BLEDevice> AvailableDevices { get; }
    }
}
