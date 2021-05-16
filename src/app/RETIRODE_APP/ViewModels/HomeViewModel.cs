using Nancy.TinyIoc;
using Plugin.Permissions;
using Plugin.Permissions.Abstractions;
using RETIRODE_APP.Services.Interfaces;
using RETIRODE_APP.Views;
using System;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.CommunityToolkit.ObjectModel;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    public class HomeViewModel : BaseViewModel
    {
        private IApplicationStateProvider _applicationStateProvider;

        public ICommand StartDepictionCommand { get; set; }
        public HomeViewModel()
        {
            _applicationStateProvider = TinyIoCContainer.Current.Resolve<IApplicationStateProvider>();
            StartDepictionCommand = new AsyncCommand(async () => await StartDepiction());
            Title = "Home";
        }

        private async Task StartDepiction()
        {

            var locationPermissionStatus = await RequestLocationPermissionIfNeeded();
            if (locationPermissionStatus != PermissionStatus.Granted)
            {
                await ShowError("Application will not work properly without location permission");
                Environment.Exit(0);
            }
            await EnsureLocationEnabled();
            await EnsureBluetoothEnabled();
            //Navigation.PushAsync(new GraphPage());

            if (!App.isConnected)
            {
                var bluetoothPage = TinyIoCContainer.Current.Resolve<BluetoothPage>();
                await Application.Current.MainPage.Navigation.PushAsync(bluetoothPage);
            }
            else if (!App.isCalibrated)
            {
                var settingsPage = TinyIoCContainer.Current.Resolve<SettingsPage>();
                await Application.Current.MainPage.Navigation.PushAsync(settingsPage);
            }
            else
            {
                var graphPage = TinyIoCContainer.Current.Resolve<GraphPage>();
                await Application.Current.MainPage.Navigation.PushAsync(graphPage);

            }
        }

        private async Task<PermissionStatus> RequestLocationPermissionIfNeeded()
        {
            var locationStatus = PermissionStatus.Unknown;
            try
            {
                locationStatus = await _applicationStateProvider.GetLocationStatus();
            }
            catch
            {
                await ShowError("Location Status problem");
            }
            return locationStatus;
        }

        public string Connected { get; set; }
    }
}