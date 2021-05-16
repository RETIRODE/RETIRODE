using Android.Content;
using Android.Locations;
using Nancy.TinyIoc;
using Plugin.Permissions;
using Plugin.Permissions.Abstractions;
using RETIRODE_APP.Services.Interfaces;
using System.Threading.Tasks;


namespace RETIRODE_APP.Services
{
    public class AnroidStateProvider : IApplicationStateProvider
    {
        private IBluetoothService _bluetoothService;
        
        public AnroidStateProvider()
        {
            _bluetoothService = TinyIoCContainer.Current.Resolve<IBluetoothService>();
        }
        public async Task<bool> IsBluetoothEnabled()
        {
           return await _bluetoothService.IsBluetoothEnabled();
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

        public async Task EnableBluetooth()
        {
            await _bluetoothService.EnableBluetooth();
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
