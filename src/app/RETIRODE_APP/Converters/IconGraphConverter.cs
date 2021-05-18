using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;
using Xamarin.Forms;

namespace RETIRODE_APP.Converters
{
    public class IconGraphConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if(value is bool) 
            {
                var isMeasurement = (bool) value;
                if (isMeasurement)
                {
                    return new FileImageSource() { File = "icon_stop.png" };
                } else
                {
                    return new FileImageSource() { File = "icon_play.png" };
                }
                
            }
            return null;
            
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return null;
        }
    }
}
