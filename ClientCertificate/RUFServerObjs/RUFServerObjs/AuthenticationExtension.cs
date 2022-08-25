using Microsoft.AspNetCore.Authentication.Certificate;
using System.Security.Cryptography.X509Certificates;

namespace RUFServerObjs
{
    public static class AuthenticationExtension
    {
        public static void ConfigureAuthetication(this IServiceCollection services)
        {

            /*
                .AddAuthentication(CertificateAuthenticationDefaults.AuthenticationScheme)
                .AddCertificate(options =>
                {
                    options.AllowedCertificateTypes = CertificateTypes.All;
                });

             */

            services.AddAuthentication(CertificateAuthenticationDefaults.AuthenticationScheme)
                .AddCertificate(options =>
                {
                    options.RevocationMode = X509RevocationMode.NoCheck;
                    options.AllowedCertificateTypes = CertificateTypes.All;
/*
                    options.ChainTrustValidationMode = X509ChainTrustMode.System;
                    options.ValidateCertificateUse = false;
                    options.ValidateValidityPeriod = false;
*/

                    options.Events = new CertificateAuthenticationEvents
                    {
                        OnCertificateValidated = context =>
                        {
                            context.Success();
                            /*
                                                        var validationService = context.HttpContext.RequestServices.GetService<CertificateValidationService>();
                                                        if (validationService != null && validationService.ValidateCertificate(context.ClientCertificate))
                                                        {
                                                            Console.WriteLine("Success");
                                                            context.Success();
                                                        }
                                                        else
                                                        {
                                                            Console.WriteLine("invalid cert");
                                                            context.Fail("invalid cert");
                                                        }
                            */

                            return Task.CompletedTask;
                        },
                        OnAuthenticationFailed = context => // Не обязательно
                        {
                            context.Fail($"Invalid certificate");
                            Console.WriteLine("Auth Failed");
                            return Task.CompletedTask;
                        },
                        OnChallenge = context =>            // Не обязательно
                        {
                            // Метод вызывается перед отправкой запроса вызывающей стороне

                            Console.WriteLine("Challenge");
                            // По умолчанию мы должны вернуть флаг выполнения задачи
                            return Task.CompletedTask;
                        }
                    };
                });

            services.AddAuthorization();
        }
    }
}
