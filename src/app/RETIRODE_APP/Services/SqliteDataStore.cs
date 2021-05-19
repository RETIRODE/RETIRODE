using SQLite;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using RETIRODE_APP.Models;
using RETIRODE_APP.Services.Interfaces;

namespace RETIRODE_APP.Services
{
    public class SqliteDataStore : IDataStore
    {
        private SQLiteAsyncConnection _connection;

        public SqliteDataStore()
        {
            string folderPath = System.Environment.GetFolderPath(System.Environment.SpecialFolder.Personal);
            string completePath = Path.Combine(folderPath, "database.db3");
            _connection = new SQLiteAsyncConnection(completePath);
        }

        public async Task<bool> AddEntityAsync<T>(T item) where T : class, new()
        {
            var result = await _connection.InsertAsync(item);

            var getEntities = await GetEntitiesAsync<T>();
            return true;
        }

        public async Task CreateTableAsync<T>() where T : class, new()
        {
            await _connection.CreateTableAsync<T>();
        }


        public async Task<IEnumerable<T>> GetEntitiesAsync<T>(bool forceRefresh = false) where T : class, new()
        {
            var items = await _connection.Table<T>().ToListAsync();
            return items;
        }

        public async Task<int> SaveCalibrationAsync(CalibrationItem item)
        {
            if (item.Id != 0)
            {
                return await _connection.UpdateAsync(item);
            }
            else
            {
                return await _connection.InsertAsync(item);
            }
        }

        public async Task<int> SaveMeasurementAsync(MeasurementItem item)
        {
            if (item.Id != 0)
            {
                return await _connection.UpdateAsync(item);
            }
            else
            {
                return await _connection.InsertAsync(item);
            }
        }

        public async Task<int> DeleteCalibrationAsync(CalibrationItem item)
        {
            return await _connection.DeleteAsync<CalibrationItem>(item);
        }
        
        public async Task<int> DeleteMeasurementAsync(MeasurementItem item)
        {
            return await _connection.DeleteAsync<MeasurementItem>(item);
        }

        public async Task<CalibrationItem> GetCalibrationItemAsync(int id)
        {
            return await _connection.Table<CalibrationItem>().Where(i => i.Id == id).FirstOrDefaultAsync();
        }

        public async Task<MeasurementItem> GetMeasurementAsync(int id)
        {
            return await _connection.Table<MeasurementItem>().Where(i => i.Id == id).FirstOrDefaultAsync();
        }

        public async Task<IEnumerable<MeasurementItem>> ListMeasurementByCalibrationAsync(int calibrationId)
        {
            var items = await _connection.Table<MeasurementItem>().Where(i => i.Calibration_id == calibrationId).ToListAsync();
            return items;
        }




    }
}
