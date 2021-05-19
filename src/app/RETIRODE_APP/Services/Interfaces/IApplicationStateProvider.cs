using Plugin.Permissions.Abstractions;
using System.Threading.Tasks;


namespace RETIRODE_APP.Services.Interfaces
{
    public interface IApplicationStateProvider
    {
        /// <summary>
        /// Open settings to provide user to enable location
        /// </summary>
        void OpenSettingsToEnableLocation();

        /// <summary>
        /// Returns true if location is enabled on mobile device,
        /// returns false if not
        /// </summary>
        /// <returns></returns>
        bool IsLocationEnabled();

        /// <summary>
        /// Returns true if bluetooth adapter is enabled on mobile device, 
        /// returns false if not
        /// </summary>
        /// <returns></returns>
        Task<bool> IsBluetoothEnabled();

        /// <summary>
        /// Returns PermissionStatus for location. Shows modal window
        /// to enable permission for using location
        /// </summary>
        /// <returns></returns>
        Task<PermissionStatus> GetLocationStatus();

        /// <summary>
        /// Enable bluetooth adapter in settings
        /// </summary>
        /// <returns></returns>
        Task EnableBluetooth();
        Task<PermissionStatus> GetStoragePermissionStatus();
    }
}
