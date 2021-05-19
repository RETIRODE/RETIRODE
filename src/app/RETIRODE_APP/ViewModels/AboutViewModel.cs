using RETIRODE_APP.Models;
using RETIRODE_APP.Views;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Xamarin.CommunityToolkit.ObjectModel;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace RETIRODE_APP.ViewModels
{
    class AboutViewModel : BaseViewModel
    {
        public IList<AboutItem> AboutItems { get; }
        public IAsyncCommand<AboutItem> ItemTapped { get; }

        public AboutViewModel()
        {
            Title = "About";

            ItemTapped = new AsyncCommand<AboutItem>(async (aboutItem) => await OnItemSelected(aboutItem));

            AboutItems = new List<AboutItem>();
            AboutItems.Add(new AboutItem
            {
                Name = "About application",
                ImageSrc = "icon_aboutorange.png",
                Link = "and it's authors"
            });
            AboutItems.Add(new AboutItem
            {
                Name = "Github repository",
                ImageSrc = "icon_github.png",
                Link = "https://github.com/RETIRODE/RETIRODE"
            });
            AboutItems.Add(new AboutItem
            {
                Name = "Team website",
                ImageSrc = "icon_team.png",
                Link = "http://retirode.epizy.com/"
            });
            AboutItems.Add(new AboutItem
            {
                Name = "ON Semiconductor",
                ImageSrc = "icon_onsemi.png",
                Link = "https://www.onsemi.com/"
            });
            AboutItems.Add(new AboutItem
            {
                Name = "FEI STU University",
                ImageSrc = "icon_university.png",
                Link = "https://www.fei.stuba.sk/"
            });
        }

        public async Task OnItemSelected(AboutItem item)
        {
            if (item == null)
                return;

            if(item.Name == "About application")
            {
                await Application.Current.MainPage.Navigation.PushAsync(new InfoPage());
            }
            else
            {
                try
                {
                    await Browser.OpenAsync(new Uri(item.Link), BrowserLaunchMode.SystemPreferred);
                }
                catch (Exception e)
                {
                    await ShowError($"Could not load {item.Name} website");
                }
            }
        }
    }
}
