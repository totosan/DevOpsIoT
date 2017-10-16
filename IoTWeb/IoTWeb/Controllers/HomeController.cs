using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using IoTWeb.Models;
using Microsoft.Azure.Devices;
using Microsoft.Extensions.Configuration;
using System.IO;
using Microsoft.Extensions.Options;

namespace IoTWeb.Controllers
{
	public class HomeController : Controller
	{
		private readonly AppSettings _appsettings;

		public HomeController(IOptions<AppSettings> appsettings)
		{
			_appsettings = appsettings.Value;
		}

		RegistryManager regMan;
		public IActionResult Index()
		{
			SetDeviceList();

			return View();
		}


		private async Task SetDeviceList()
		{
			try
			{
				regMan = RegistryManager.CreateFromConnectionString($"{_appsettings.IoTHubConnectionString}");
				var devices = await regMan.GetDevicesAsync(10);

				ViewBag.Devices = devices.ToList();
			}
			catch (Exception ex)
			{

				throw;
			}
		}

		public IActionResult Overview()
		{
			ViewData["Message"] = "See details here...";

			return View();
		}

		public IActionResult About()
		{
			ViewData["Message"] = "Your application description page.";

			return View();
		}

		public IActionResult Contact()
		{
			ViewData["Message"] = "Your contact page.";

			return View();
		}

		public IActionResult Error()
		{
			return View(new ErrorViewModel { RequestId = Activity.Current?.Id ?? HttpContext.TraceIdentifier });
		}
	}
}
