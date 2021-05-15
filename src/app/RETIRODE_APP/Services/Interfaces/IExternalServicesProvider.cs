using RETIRODE_APP.Models.Enums;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services.Interfaces
{
    public interface IExternalServicesProvider
    {
        void OpenSettingsToEnableLocation();
        bool IsLocationEnabled();
        bool IsBluetoothEnabled();
        Task<bool> IsLocationPermissionGranted();

        void EnableBluetooth();
    }
}
