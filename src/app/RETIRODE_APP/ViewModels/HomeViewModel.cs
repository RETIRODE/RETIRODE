using Nancy.TinyIoc;
using RETIRODE_APP.Models.Enums;
using RETIRODE_APP.Services.Interfaces;
using System;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    public class HomeViewModel : BaseViewModel
    {
        private IExternalServicesProvider _externalServiceProvider;
        public HomeViewModel()
        {
            _externalServiceProvider = TinyIoCContainer.Current.Resolve<IExternalServicesProvider>();
            Title = "Home";
            ckeckConnection();
            CheckLocation();
            CheckBluetooth();
        }

        public void ckeckConnection()
        {
            if (App.isConnected)
                Connected = "Connected";
            else
                Connected = "Disconnected";
        }

        private async void CheckLocation()
        {
 
            if (!await _externalServiceProvider.IsLocationPermissionGranted())
            {
                if(await ShowDialog("Enable location permission?"))
                {
                    await Permissions.RequestAsync<Permissions.LocationAlways>();
                }
                else
                {
                    Environment.Exit(0);
                }

                if (!_externalServiceProvider.IsLocationEnabled())
                {
                    if(await ShowDialog("Enable location?"))
                    {
                        _externalServiceProvider.OpenSettingsToEnableLocation();
                    }
                    else
                    {
                        Environment.Exit(0);
                    }
                }
            }
            else
            {
                if (!_externalServiceProvider.IsLocationEnabled())
                {
                    _externalServiceProvider.OpenSettingsToEnableLocation();
                }
            }
        }

        private void CheckBluetooth()
        {
            if(!_externalServiceProvider.IsBluetoothEnabled())
            {
                externalServiceProvider.EnableBluetooth();
            }
        }

        public string Connected { get;set; }
    }
}