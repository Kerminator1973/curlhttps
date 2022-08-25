using Microsoft.AspNetCore.Authentication.Certificate;
using Microsoft.AspNetCore.Server.Kestrel.Https;
using RUFServerObjs;
using System.Security.Cryptography.X509Certificates;

var builder = WebApplication.CreateBuilder(args);

// ����������� Kestrel ����� �������, ����� ���������� ���������� ����������
builder.WebHost.ConfigureKestrel(serverOptions =>
{
    serverOptions.ConfigureHttpsDefaults(o =>
    {
        o.ClientCertificateMode = ClientCertificateMode.RequireCertificate;
    });
});

// ����������� ������������ ����������� (Dependency Injections)
//builder.WebHost.ConfigureServices(services => {
builder.Services.AddTransient<CertificateValidationService>();
builder.Services.ConfigureAuthetication();
//});


// Add services to the container.

var app = builder.Build();

// Configure the HTTP request pipeline.

// ���������� �������������� � ����������� ��� ���������� �������
app.UseAuthentication();    // ��������� �������������� (��.: https://mydevtricks.com/certificate-authentication-in-aspnet-core)
app.UseAuthorization();     // ����������� ����� - �� �������� ��������������� ��������� �� ������, ����� �������� ����� ����� �������

//app.UseHttpsRedirection();


var summaries = new[]
{
    "Freezing", "Bracing", "Chilly", "Cool", "Mild", "Warm", "Balmy", "Hot", "Sweltering", "Scorching"
};

// ���������� ����� ����� (HTTP GET) � ���, ������������ ��������� ��������� �����
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
;  // ��� ���� �����, ��� � [Authorize] () => "This endpoint requires authorization."

app.Run();

internal record WeatherForecast(DateTime Date, int TemperatureC, string? Summary)
{
    public int TemperatureF => 32 + (int)(TemperatureC / 0.5556);
}

// �����, ������������ "���������" ������������ ���������� �� ������� � ������ �� ���������� �����
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
