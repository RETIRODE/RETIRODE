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
        /// <returns>
        /// True - location is enabled
        /// False - location is disabled
        /// </returns>
        bool IsLocationEnabled();

        /// <summary>
        /// Returns true if bluetooth adapter is enabled on mobile device, 
        /// returns false if not
        /// </summary>
        /// <returns>
        /// True - bluetooth is enabled
        /// False - bluetooth is disabled
        /// </returns>
        Task<bool> IsBluetoothEnabled();

        /// <summary>
        /// Returns PermissionStatus for location. Shows modal window
        /// to enable permission for using location
        /// </summary>
        /// <returns>
        /// PermissionStatus.Granted - location is enabled
        /// PermissionStatus.Denied - location is disabled
        /// </returns>
        Task<PermissionStatus> GetLocationStatus();

        /// <summary>
        /// Enable bluetooth adapter in mobile device
        /// </summary>
        /// <returns></returns>
        Task EnableBluetooth();
        Task<PermissionStatus> GetStoragePermissionStatus();
    }
}
