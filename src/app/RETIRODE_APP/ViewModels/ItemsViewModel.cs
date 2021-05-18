using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services.Interfaces;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.CommunityToolkit.ObjectModel;

namespace RETIRODE_APP.ViewModels
{
    public class ItemsViewModel : BaseViewModel
    {
        private CalibrationItem _selectedItem;

        private readonly IDataStore _dataStore;

        public ObservableCollection<CalibrationItem> Items { get; }
        public ICommand LoadItemsCommand { get; }
        public ICommand ItemTapped { get; }

        public ItemsViewModel()
        {
            Title = "Scan History";
            _dataStore = TinyIoCContainer.Current.Resolve<IDataStore>();
            Items = new ObservableCollection<CalibrationItem>();
            LoadItemsCommand = new AsyncCommand(async () => await LoadItemsAsync());
            ItemTapped = new AsyncCommand(async () => await OnItemSelected());
        }

        private async Task LoadItemsAsync()
        {

            try
            {
                Items.Clear();



                var list = new List<CalibrationItem>(){ new CalibrationItem()
            {
                DateTime = DateTime.Now,
                Id = 1,
                Pulse_count = 100,
                Tdc_0 = 4,
                Tdc_62 = 659,
                Tdc_125 = 1272
            },
            new CalibrationItem{
                DateTime = DateTime.Now,
                Id = 1,
                Pulse_count = 100,
                Tdc_0 = 4,
                Tdc_62 = 659,
                Tdc_125 = 1272
            }};// await _dataStore.GetEntitiesAsync<CalibrationItem>();

                foreach (var item in list)
                {
                    Items.Add(item);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
            }
            finally
            {
                IsBusy = false;
            }
        }

        public CalibrationItem SelectedItem
        {
            get => _selectedItem;
            set
            {
                SetProperty(ref _selectedItem, value);
            }
        }

        private async Task OnItemSelected()
        {
            //TODO: Push graph page
        }
    }
}