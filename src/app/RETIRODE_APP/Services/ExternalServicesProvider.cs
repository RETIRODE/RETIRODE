using Android.Bluetooth;
using Android.Content;
using Android.Locations;
using RETIRODE_APP.Models.Enums;
using RETIRODE_APP.Services.Interfaces;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Essentials;

namespace RETIRODE_APP.Services
{
    public class ExternalServicesProvider : IExternalServicesProvider
    {
        public bool IsBluetoothEnabled()
        {
            BluetoothManager bluetoothManager = (BluetoothManager)Android.App.Application.Context.GetSystemService(Context.BluetoothService);
            return bluetoothManager.Adapter.IsEnabled;
        }

        public bool IsLocationEnabled()
        {
            LocationManager locationManager = (LocationManager)Android.App.Application.Context.GetSystemService(Context.LocationService);
            return locationManager.IsProviderEnabled(LocationManager.GpsProvider);
        }

        public async Task<bool> IsLocationPermissionGranted()
        {
            var locationPermissionStatus = await Permissions.CheckStatusAsync<Permissions.LocationAlways>();

            if (locationPermissionStatus != PermissionStatus.Granted)
            {
                return false;
            }
            return true;
        }

        public void EnableBluetooth()
        {
            BluetoothManager bluetoothManager = (BluetoothManager)Android.App.Application.Context.GetSystemService(Context.BluetoothService);
            bluetoothManager.Adapter.Enable();
        }
    
        public void OpenSettingsToEnableLocation()
        {
            Intent intent = new Intent(Android.Provider.Settings.ActionLocationSourceSettings);
            intent.SetFlags(ActivityFlags.ClearTop | ActivityFlags.NewTask);

            try
            {
                Android.App.Application.Context.StartActivity(intent);
            }
            catch(ActivityNotFoundException ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
        }
    }
}
