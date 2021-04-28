using System;

namespace RETIRODE_APP.Helpers
{
    public static class Helper
    {
        public static void NullCheck(object obj)
        {
            if(obj is null)
            {
                throw new NullReferenceException(nameof(obj));
            }
        }
    }
}
