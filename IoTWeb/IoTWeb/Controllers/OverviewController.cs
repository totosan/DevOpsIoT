using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text.Encodings.Web;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;

// For more information on enabling MVC for empty projects, visit https://go.microsoft.com/fwlink/?LinkID=397860

namespace IoTWeb.Controllers
{
    public class OverviewController : Controller
    {
        // GET: /Overview/
        public IActionResult Index()
        {
			return View();
        }

		public IActionResult Functions(string deviceId)
		{
			ViewData["deviceId"] = deviceId;

			return View();
		}

		public async Task<IActionResult> TurnOn(string deviceId)
		{
			var url = "https://devops005function.azurewebsites.net/api/TurnFan?action=On&deviceId="+deviceId;
			var client = new HttpClient();

			var response = await client.GetAsync(url);
			var content = await response.Content.ReadAsStringAsync();

			return View("Functions");
		}
    }
}
