using SQLite;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;
using System.Linq;

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

            var getEntities = GetEntitiesAsync<T>();
            return true;
        }

        public async Task CreateTableAsync<T>() where T : class, new()
        {
            await _connection.CreateTableAsync<T>();
        }

        public Task<T> GetEntityAsync<T>(int id) where T : class, new()
        {
            throw new NotImplementedException();
        }

        public async Task<IEnumerable<T>> GetEntitiesAsync<T>(bool forceRefresh = false) where T : class, new()
        {
            var items = await _connection.Table<T>().ToListAsync();
            return items;
        }

        public Task<bool> UpdateEntityAsync<T>(T item) where T : class, new()
        {
            throw new NotImplementedException();
        }

        public Task<bool> DeleteEntityAsync<T>(int id) where T : class, new()
        {
            throw new NotImplementedException();
        }
    }
}
