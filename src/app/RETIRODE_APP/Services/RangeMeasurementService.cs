using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public class RangeMeasurementService : IRangeMeasurementService
    {
        public IEnumerable<BLEDevice> AvailableDevices()
        {
            // TEMPORARY MOCKED DATA
            List<BLEDevice> devices = new List<BLEDevice>();
            Random random = new Random();
            for(int i = 0; i < 2; i++)
            {
                devices.Add(new BLEDevice()
                {
                    Name = "asd" + i,
                    Identifier = Guid.NewGuid()
                });
            }
            return devices;
        }

        public bool CalibrateLIDAR(RSL10Command command)
        {
            return true;
        }

        public Task<bool> ConnectToRSL10(BLEDevice bleDevice)
        {
            return Task.FromResult(true);
        }

        public Task<bool> DisconnectFromRSL10()
        {
            return Task.FromResult(true);
        }

        public bool StartMeasurement(RSL10Command command)
        {
            return true;
        }

        public bool StopMeasurement(RSL10Command command)
        {
            return true;
        }
    }
}
