﻿using Nancy.TinyIoc;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services.Interfaces;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.CommunityToolkit.ObjectModel;

namespace RETIRODE_APP.ViewModels
{
    class GraphViewModel : BaseViewModel
    {
        private IDataStore _dataStore;
        private IRangeMeasurementService _rangeMeasurementService;
        
        private DateTime StartMeasuringTime { get; set; }
        private CalibrationItem Calibration { get; set; }
        public ObservableCollection<MeasuredDataItem> MeasuredDataItems { get; set; }
        public ICommand StartStopCommand { get; set; }
        public ICommand GraphResetCommand { get; set; }
        public string CurrentIcon { get; set; }

        private bool _measurement;
        public bool Measurement
        {
            get { return _measurement; }
            set
            {
                _measurement = value;
                OnPropertyChanged(nameof(Measurement));
            }
        }

        public GraphViewModel()
        {
            StartMeasuringTime = DateTime.Now;
            MeasuredDataItems = new ObservableCollection<MeasuredDataItem>();
            StartStopCommand = new AsyncCommand(async () => await StartStopMeasurement());
            GraphResetCommand = new AsyncCommand(async () => await GraphReset());
            _rangeMeasurementService = TinyIoCContainer.Current.Resolve<IRangeMeasurementService>();
            _dataStore = TinyIoCContainer.Current.Resolve<IDataStore>();
            _rangeMeasurementService.MeasuredDataResponseEvent -= _rangeMeasurementService_MeasuredDataResponseEvent;
            _rangeMeasurementService.MeasuredDataResponseEvent += _rangeMeasurementService_MeasuredDataResponseEvent;
            Measurement = true;
            _rangeMeasurementService.StartMeasurement();
        }

        private Task GraphReset()
        {
            MeasuredDataItems.Clear();
            return Task.CompletedTask;
        }

        public async Task StartStopMeasurement()
        {
            
            if (Measurement)
            {
               // await WithBusy(() => _rangeMeasurementService.StopMeasurement());
                Measurement = false;
            } else
            {
                //await WithBusy(() => _rangeMeasurementService.StartMeasurement());
                Measurement = true;
            }
            
        }

        public async Task StartMeasurement()
        {
            Measurement = true;
            await _rangeMeasurementService.StartMeasurement();
        }

        private async void _rangeMeasurementService_MeasuredDataResponseEvent(List<float> obj)
        {
            int i = 0;
            foreach (var item in obj)
            {
                i++;
                var distance = CalculateDistanceFromTdc(item);
                TimeSpan span = (DateTime.Now - StartMeasuringTime);
                MeasuredDataItems.Add(new MeasuredDataItem(distance, (float)span.TotalMilliseconds + TimeSpan.FromMilliseconds(100 * i).Milliseconds));
                OnPropertyChanged(nameof(MeasuredDataItems));

                await _dataStore.AddEntityAsync(new MeasurementItem()
                {
                    Calibration_id = Calibration.Id,
                    Tdc_value = item
                });
            }
        }

        public async void Init()
        {
            await SetCalibration();
          //  await LoadValues();
        }

        private async Task LoadValues()
        {
            var list = await _dataStore.ListMeasurementByCalibrationAsync(Calibration.Id);

            foreach (var item in list)
            {
                TimeSpan span = DateTime.Now - StartMeasuringTime;
                var distance = CalculateDistanceFromTdc(item.Tdc_value);
                MeasuredDataItems.Add(new MeasuredDataItem(distance, (float)span.TotalMilliseconds));
                OnPropertyChanged(nameof(MeasuredDataItems));
            }
        }

        private float CalculateDistanceFromTdc(float tdcValue)
        {
            float tofValue;

            if (tdcValue < 0)
            {
                tofValue = 0f;
            }
            else if (tdcValue < 62.5f)
            {
                tofValue = 0f + (62.5f * ((tdcValue - Calibration.Tdc_0) / (Calibration.Tdc_62 - Calibration.Tdc_0)));
            }
            else if (tdcValue < 125f)
            {
                tofValue = 62.5f + (62.5f * ((tdcValue - Calibration.Tdc_62) / (Calibration.Tdc_125 - Calibration.Tdc_62)));
            }
            else
            {
                tofValue = 125f + (62.5f * ((tdcValue - Calibration.Tdc_125) / (Calibration.Tdc_125 - Calibration.Tdc_62)));
            }

            float distance = 0.15f * tofValue;
            return distance;
        }
        private async Task SetCalibration()
        {
            var calibrationList = await _dataStore.GetEntitiesAsync<CalibrationItem>();
            var calibration = new List<CalibrationItem>(calibrationList).FindLast(x => x.Id > 0);
            Calibration = calibration;
        }

    }
}
