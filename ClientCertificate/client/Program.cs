using System.Net;
using System.Security.Cryptography.X509Certificates;

// Для доступа к сайтам в интернет, необходимо указать Proxy
var proxy = new WebProxy
{
    Address = new Uri($"http://192.168.100.200:3128"),
    BypassProxyOnLocal = false,
    UseDefaultCredentials = true,
};

// Указываем, что следует идти через прокси
var handler = new HttpClientHandler()
{
    //Proxy = proxy,
};

// Добавляем клиентский сертификат, который был нам предоставлен сайтом badssl.com
var cert = new X509Certificate2("c:/Temp/dorsfull.pfx", "1234");
//var cert = new X509Certificate2("c:/Temp/badssl.com-client.p12", "badssl.com");
handler.ClientCertificates.Add(cert);

var client = new HttpClient(handler);

// Описываем запрос по https
var request = new HttpRequestMessage()
{
    RequestUri = new Uri("https://localhost:8081/weatherforecast"),
    //RequestUri = new Uri("https://client.badssl.com"),
    Method = HttpMethod.Get,
};

// Выполняем запрос 
var response = await client.SendAsync(request);
if (response.IsSuccessStatusCode)
{
    var responseContent = response.Content.ReadAsStream();
    Console.WriteLine(new StreamReader(responseContent).ReadToEnd());
}
