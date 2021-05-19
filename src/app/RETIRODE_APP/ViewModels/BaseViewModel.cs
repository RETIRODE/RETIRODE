using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services.Interfaces;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    public class BaseViewModel : INotifyPropertyChanged
    {
        
        protected readonly IApplicationStateProvider _applicationStateProvider;

        

        bool isBusy = false;
        public bool IsBusy
        {
            get { return isBusy; }
            set { SetProperty(ref isBusy, value); }
        }

        string title = string.Empty;


        public string Title
        {
            get { return title; }
            set { SetProperty(ref title, value); }
        }

        public BaseViewModel()
        {
            _applicationStateProvider = TinyIoCContainer.Current.Resolve<IApplicationStateProvider>();
        }

        protected async Task WithBusy(Func<Task> action)
        {
            bool completed = false;
            try
            {
                ShowBusy(() => completed, 5); ;
                await action();
                completed = true;
            }
            catch (Exception)
            {
                throw;
            }
            finally
            {
                completed = true;
            }
        }


        protected bool SetProperty<T>(ref T backingStore, T value,
            [CallerMemberName] string propertyName = "",
            Action onChanged = null)
        {
            if (EqualityComparer<T>.Default.Equals(backingStore, value))
                return false;

            backingStore = value;
            onChanged?.Invoke();
            OnPropertyChanged(propertyName);
            return true;
        }

        protected async Task ShowError(string message)
        {
            await Application.Current.MainPage.DisplayAlert("Error", message, "OK");
        }

        protected void ShowBusy(Func<bool> func, int timeout)
        {

            var tokenSource = new CancellationTokenSource(timeout * 1000);
            IsBusy = true;
            Task.Run(() =>
            {
                try
                {
                    while (!tokenSource.IsCancellationRequested)
                    {
                        if (func())
                        {
                            break;
                        }
                    }
                }
                finally
                {
                    IsBusy = false;
                    tokenSource.Dispose();
                }
            }, tokenSource.Token);
        }

        protected async Task<bool> ShowDialog(string message)
        {
            return await Application.Current.MainPage.DisplayAlert("Question", message, "Yes", "No");
        }

        protected async Task EnsureLocationEnabled()
        {
            if (!_applicationStateProvider.IsLocationEnabled())
            {
                if (await ShowDialog("Enable location?"))
                {
                    _applicationStateProvider.OpenSettingsToEnableLocation();
                }
                else
                {
                    Environment.Exit(0);
                }
            }
        }

        protected async Task EnsureBluetoothEnabled()
        {
            if (!await _applicationStateProvider.IsBluetoothEnabled())
            {
                if (await ShowDialog("Bluetooth is turned off. Do you want to enable Bluetooth?"))
                {
                    await _applicationStateProvider.EnableBluetooth();
                }
                else
                {
                    await ShowDialog("Bluetooth must be turn on");
                    Environment.Exit(0);
                }
            }
        }
        #region INotifyPropertyChanged
        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string propertyName = "")
        {
            var changed = PropertyChanged;
            if (changed == null)
                return;

            changed.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
        #endregion
    }
}
