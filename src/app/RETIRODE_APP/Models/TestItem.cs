using SQLite;
using System;
using System.Collections.Generic;
using System.Text;

namespace RETIRODE_APP.Models
{
    class TestItem
    {
        public TestItem()
        {
        }

        [PrimaryKey, AutoIncrement]
        public int Id { get; set; }
        public string Text { get; set; }
        public string Description { get; set; }
    }

}
