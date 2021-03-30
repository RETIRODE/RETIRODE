using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public interface IRangeMeasurementService
    {
        bool StartMeasurement(RSL10Command command);

        bool StopMeasurement(RSL10Command command);

        Task<bool> ConnectToRSL10(BLEDevice bleDevice);

        IEnumerable<BLEDevice> AvailableDevices();

        bool CalibrateLIDAR(RSL10Command command);

        Task<bool> DisconnectFromRSL10();
    }
}
