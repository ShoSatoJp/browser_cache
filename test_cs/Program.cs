using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
namespace test_cs
{
	class Program
	{
		static void Main(string[] args)
		{
			string path = @"C:\Users\User\AppData\Local\Google\Chrome\User Data\Profile 3\Cache\data_0";
			using (FileStream fs = File.Open(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite)) {
				using (StreamReader sr = new StreamReader(fs)) {
					String str = sr.ReadLine();
					Console.WriteLine(str);
				}
			}

		}
	}
}
