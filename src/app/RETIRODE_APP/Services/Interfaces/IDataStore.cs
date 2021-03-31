using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace RETIRODE_APP.Services
{
    public interface IDataStore
    {
        Task<bool> AddEntityAsync<T>(T item) where T : class, new();
        Task<bool> UpdateEntityAsync<T>(T item) where T : class, new();
        Task<bool> DeleteEntityAsync<T>(int id) where T : class, new();
        Task<T> GetEntityAsync<T>(int id) where T : class, new();
        Task<IEnumerable<T>> GetEntitiesAsync<T>(bool forceRefresh = false) where T : class, new();
        Task CreateTableAsync<T>() where T : class, new();

    }
}
