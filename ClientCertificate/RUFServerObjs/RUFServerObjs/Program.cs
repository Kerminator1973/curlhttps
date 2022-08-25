using Microsoft.AspNetCore.Authentication.Certificate;
using Microsoft.AspNetCore.Server.Kestrel.Https;
using RUFServerObjs;
using System.Security.Cryptography.X509Certificates;

var builder = WebApplication.CreateBuilder(args);

// Настраиваем Kestrel таким образом, чтобы требовался клиентский сертификат
builder.WebHost.ConfigureKestrel(serverOptions =>
{
    serverOptions.ConfigureHttpsDefaults(o =>
    {
        o.ClientCertificateMode = ClientCertificateMode.RequireCertificate;
    });
});

// Настраиваем используемые зависимости (Dependency Injections)
//builder.WebHost.ConfigureServices(services => {
builder.Services.AddTransient<CertificateValidationService>();
builder.Services.ConfigureAuthetication();
//});


// Add services to the container.

var app = builder.Build();

// Configure the HTTP request pipeline.

// Активируем аутентификацию и авторизацию при выполнении запроса
app.UseAuthentication();    // Добавляем аутентификацию (см.: https://mydevtricks.com/certificate-authentication-in-aspnet-core)
app.UseAuthorization();     // Авторизация нужна - мы получаем соответствующее сообщение об ошибке, когда пытаемся зайти через браузер

//app.UseHttpsRedirection();


var summaries = new[]
{
    "Freezing", "Bracing", "Chilly", "Cool", "Mild", "Warm", "Balmy", "Hot", "Sweltering", "Scorching"
};

// Определяем точку входа (HTTP GET) и код, генерирующий некоторый случайный ответ
app.MapGet("/weatherforecast", () =>
{
    var forecast = Enumerable.Range(1, 5).Select(index =>
        new WeatherForecast
        (
            DateTime.Now.AddDays(index),
            Random.Shared.Next(-20, 55),
            summaries[Random.Shared.Next(summaries.Length)]
        ))
        .ToArray();
    return forecast;
//});
}).RequireAuthorization()
;  // Это тоже самое, что и [Authorize] () => "This endpoint requires authorization."

app.Run();

internal record WeatherForecast(DateTime Date, int TemperatureC, string? Summary)
{
    public int TemperatureF => 32 + (int)(TemperatureC / 0.5556);
}

// Метод, сравнивающий "отпечатки" сертификатов полученные от клиента и взятые из указанного файла
public class CertificateValidationService
{
    public bool ValidateCertificate(X509Certificate2 clientCertificate)
    {
        var cert = new X509Certificate2(Path.Combine("c://Temp//test-cert.pfx"), "1234");
        if( clientCertificate.Thumbprint == cert.Thumbprint )
        {
            return true;
        }

        return false;
    }
}
