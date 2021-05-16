using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services.Interfaces;
using System;
using System.Collections.Generic;

using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    public class NewItemViewModel : BaseViewModel
    {
        private string text;
        private string description;

        private IDataStore _itemDataStore => TinyIoCContainer.Current.Resolve<IDataStore>();

        public NewItemViewModel()
        {
            SaveCommand = new Command(OnSave, ValidateSave);
            CancelCommand = new Command(OnCancel);
            this.PropertyChanged +=
                (_, __) => SaveCommand.ChangeCanExecute();
        }

        private bool ValidateSave()
        {
            return !String.IsNullOrWhiteSpace(text)
                && !String.IsNullOrWhiteSpace(description);
        }

        public string Text
        {
            get => text;
            set => SetProperty(ref text, value);
        }

        public string Description
        {
            get => description;
            set => SetProperty(ref description, value);
        }

        public Command SaveCommand { get; }
        public Command CancelCommand { get; }

        private async void OnCancel()
        {
            // This will pop the current page off the navigation stack
            await Shell.Current.GoToAsync("..");
        }

        private async void OnSave()
        {
            //TestItem newItem = new TestItem()
            //{
            //    Text = Text,
            //    Description = Description
            //};

            //await _itemDataStore.CreateTableAsync<TestItem>();
            //await _itemDataStore.AddEntityAsync<TestItem>(newItem);

            CalibrationItem calibrationItem = new CalibrationItem()
            {
                Tdc_0 = 4f,
                Tdc_62 = 738f,
                Tdc_125 = 1453f,
                DateTime = DateTime.Now
            };
            await _itemDataStore.AddEntityAsync<CalibrationItem>(calibrationItem);
            var calibrationList = await _itemDataStore.GetEntitiesAsync<CalibrationItem>();
            var calibration = new List<CalibrationItem>(calibrationList).FindLast(x => x.Id > 0);

            var list = new List<MeasurementItem>();
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 16.519073f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 16.604225f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 16.008175f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 16.774525f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 16.689373f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 6.897139f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 6.471389f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 17.200272f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 16.774525f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 17.029972f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 16.263624f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 17.626020f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 20.861717f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 21.968664f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 22.224115f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 20.606266f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 20.010218f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 17.966621f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 17.881472f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 17.285421f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 17.881472f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 17.200272f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 16.944824f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 16.689373f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 5.108992f });
            list.Add(new MeasurementItem(){ Calibration_id = calibration.Id, Tdc_value = 5.023842f });

            foreach (var item in list)
            {
                await _itemDataStore.AddEntityAsync<MeasurementItem>(item);
            }

            var res = await _itemDataStore.GetMeasurementAsync(calibration.Id);


            // This will pop the current page off the navigation stack
            await Shell.Current.GoToAsync("..");
        }
    }
}
