using Android.Bluetooth;
using Android.Content;
using Android.Locations;
using Nancy.TinyIoc;
using Plugin.Permissions;
using Plugin.Permissions.Abstractions;
using RETIRODE_APP.Services.Interfaces;
using System.Threading.Tasks;


namespace RETIRODE_APP.Services
{
    public class ApplicationStateProvider : IApplicationStateProvider
    {
        private IBluetoothService _bluetoothService;
        
        public ApplicationStateProvider()
        {
            _bluetoothService = TinyIoCContainer.Current.Resolve<IBluetoothService>();
        }
        public Task<bool> IsBluetoothEnabled()
        {
            BluetoothManager bluetoothManager = (BluetoothManager)Android.App.Application.Context.GetSystemService(Context.BluetoothService);
            var res = bluetoothManager.Adapter.State == State.On;
            return Task.FromResult(res);
        }

        public bool IsLocationEnabled()
        {
            LocationManager locationManager = (LocationManager)Android.App.Application.Context.GetSystemService(Context.LocationService);
            return locationManager.IsLocationEnabled;
        }

        public async Task<PermissionStatus> GetLocationStatus()
        {
            try
            {
                var locationStatus = await CrossPermissions.Current.CheckPermissionStatusAsync<LocationPermission>();

                if (locationStatus != PermissionStatus.Granted)
                {
                    return await CrossPermissions.Current.RequestPermissionAsync<LocationPermission>();
                }
                return locationStatus;

            }
            catch
            {
                throw;
            }
            
        }

        public Task EnableBluetooth()
        {
            BluetoothManager bluetoothManager = (BluetoothManager)Android.App.Application.Context.GetSystemService(Context.BluetoothService);
            bluetoothManager.Adapter.Enable();
            return Task.CompletedTask;
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

        public async Task<PermissionStatus> GetStoragePermissionStatus()
        {
            try
            {
                var storageStatus = await CrossPermissions.Current.CheckPermissionStatusAsync<StoragePermission>();
                if (storageStatus != PermissionStatus.Granted)
                {
                    return await CrossPermissions.Current.RequestPermissionAsync<StoragePermission>();
                }
                return storageStatus;
            } 
            catch
            {
                throw;
            }
        }
    }
}
