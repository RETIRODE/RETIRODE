using RETIRODE_APP.Models;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public interface IDataStore
    {
        Task<IEnumerable<MeasurementItem>> ListMeasurementByCalibrationAsync(int calibrationId);
        Task<CalibrationItem> GetCalibrationItemAsync(int id);
        Task<MeasurementItem> GetMeasurementAsync(int id);
        Task<bool> AddEntityAsync<T>(T item) where T : class, new();
        Task<int> SaveCalibrationAsync(CalibrationItem item);
        Task<int> SaveMeasurementAsync(MeasurementItem item);
        Task<int> DeleteCalibrationAsync(CalibrationItem item);
        Task<int> DeleteMeasurementAsync(MeasurementItem item);
        Task<IEnumerable<T>> GetEntitiesAsync<T>(bool forceRefresh = false) where T : class, new();
        Task CreateTableAsync<T>() where T : class, new();

    }
}
