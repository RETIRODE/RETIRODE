using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public interface IRangeMeasurementService
    {
        Task<bool> StartMeasurement();

        Task<bool> StopMeasurement();

        Task<bool> ConnectToRSL10(BLEDevice bleDevice);

        Task StartScanning();

        Task<bool> CalibrateLIDAR();

        Task<bool> Disconnect(BLEDevice bleDevice);

        IList<BLEDevice> AvailableDevices { get; }
    }
}
