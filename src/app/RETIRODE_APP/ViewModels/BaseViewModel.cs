using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services;
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
        public IMockDataStore<Item> DataStore => TinyIoCContainer.Current.Resolve<IMockDataStore<Item>>();

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
