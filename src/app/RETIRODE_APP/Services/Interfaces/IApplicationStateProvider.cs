using Plugin.Permissions.Abstractions;
using System.Threading.Tasks;


namespace RETIRODE_APP.Services.Interfaces
{
    public interface IApplicationStateProvider
    {
        void OpenSettingsToEnableLocation();
        bool IsLocationEnabled();
        Task<bool> IsBluetoothEnabled();
        Task<PermissionStatus> GetLocationStatus();
        Task EnableBluetooth();
        Task<PermissionStatus> GetStoragePermissionStatus();
    }
}
